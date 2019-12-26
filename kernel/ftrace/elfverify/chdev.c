#include <linux/version.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/limits.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/limits.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif
#include <linux/miscdevice.h>
#include <linux/random.h>

#include "chdev.h"
#include "utils.h"

#define DEV_NAME	"elf_verifier"

typedef struct __elf_verify_req__ {
	pid_t pid;
	int result;
	struct list_head list;
	char full_path[0];
} elf_verifier_req;

static struct elf_ver_dev_t {
	struct miscdevice misc;
	pid_t controller;
	struct mutex lock;
	struct list_head requests;
	wait_queue_head_t resp_wq;
	wait_queue_head_t reqs_wq;
} elf_ver_dev;

int _dev_init = 0;

static int misc_dev_open(struct inode* si, struct file* filp)
{
	if (elf_ver_dev.controller)
		return -EACCES;

	elf_ver_dev.controller = current->pid;
	return 0;
}

static int misc_dev_release(struct inode* si, struct file* filp)
{
	elf_ver_dev.controller = 0;
	return 0;
}

static int read_requests_locked(int* total, char* kbuf, size_t size)
{
	elf_verifier_req* req;
	char *p = kbuf;
	mutex_lock(&elf_ver_dev.lock);
	list_for_each_entry(req, &elf_ver_dev.requests, list) {
		if (req->result != VERIFY_UNKNOWN)
			continue;

		*total = (*total) + 1;;
		if (strlen(req->full_path) + 1 + p > kbuf + size)
			break;
		strcpy(p, req->full_path);
		p += strlen(p) + 1;
	}
	mutex_unlock(&elf_ver_dev.lock);
	return p - kbuf;
}

static ssize_t misc_dev_read(struct file* filp, char __user* buf, size_t size, loff_t* offset)
{
	DECLARE_WAITQUEUE(wait, current);
	char *kbuf = NULL;
	int n = 0;
	int total = 0;

	if (size <= 1)
		return -EINVAL;

	kbuf = kzalloc(size, GFP_KERNEL);
	if (unlikely(kbuf == 0))
		return -ENOSPC;

	add_wait_queue(&elf_ver_dev.reqs_wq, &wait);

	for (;;) {
		total = 0;
		n = read_requests_locked(&total, kbuf, size);
		if (n) // data read out
			break;

		if (total == 0) { // no data
			if (filp->f_flags & O_NONBLOCK) {
				n = -EAGAIN;
				goto readout;
			}

			__set_current_state(TASK_INTERRUPTIBLE);
			//TODO: better to move unlock here (quite some code needs modification then)
			schedule();
		} else if (n == 0) { // has data but buf too small
			n = 0;
			goto readout;
		}

		if (signal_pending(current)) {
			n = -EINTR;
			goto readout;
		}
	}

	if (copy_to_user(buf, kbuf, n))
		n = -EFAULT;

readout:
	remove_wait_queue(&elf_ver_dev.reqs_wq, &wait);
	set_current_state(TASK_RUNNING);
	kfree(kbuf);
	kbuf = NULL;
	return n;
}

//test: echo -e -n "\x00/media/sf_D_DRIVE/raphael/temp/kernel-enh/mylib.so\x00" > /dev/elf_verifier
static ssize_t misc_dev_write(struct file* filp, const char __user* buf, size_t size, loff_t* offset)
{
	elf_verifier_req* req = NULL;
	char *kbuf = NULL;
	char *p = NULL;
	int total = 0;

	if (size <= sizeof(unsigned char) + 1)
		return -EINVAL;

	kbuf = kzalloc(size, GFP_KERNEL);
	if (unlikely(kbuf == 0))
		return -ENOSPC;

	if (copy_from_user(kbuf, buf, size)) {
		kfree(kbuf);
		return -EFAULT;
	}

	if (kbuf[size-1] != 0) {
		kfree(kbuf);
		return -EINVAL;
	}

	p = kbuf;
	mutex_lock(&elf_ver_dev.lock);
	while (p < kbuf + size) {
		unsigned char result = *(unsigned char*)p;
		p++;
		if (result < VERIFY_OK || result >= VERIFY_UNKNOWN) {
			p += strlen(p) + 1;
			continue;
		}

		list_for_each_entry(req, &elf_ver_dev.requests, list) {
			if (strcmp(req->full_path, p) || req->result != VERIFY_UNKNOWN)
				continue;

			req->result = result;
			total++;
		}
		p += strlen(p) + 1;
	}
	mutex_unlock(&elf_ver_dev.lock);

	kfree(kbuf);
	if (total)
		wake_up_interruptible(&elf_ver_dev.resp_wq);
	return size;
}

static struct file_operations misc_dev_fops = {
	.owner = THIS_MODULE,
	.open = misc_dev_open,
	.read = misc_dev_read,
	.write = misc_dev_write,
	.release = misc_dev_release
};

static elf_verifier_req* create_evr(const char* root, const char* path,
				    unsigned int major, unsigned int minor)
{
	int path_size = 0;
	char uuid[64] = {0};
	elf_verifier_req *evr = NULL;

	memset(uuid, 0 , 64);
	if (likely(current)) {
		snprintf(uuid, 64, "%u-", current->pid);
	} else {
		snprintf(uuid, 64, "%u-", get_random_u32());
	}
	path_size = strlen(uuid) + strlen(path);

	evr = kzalloc(sizeof(elf_verifier_req) + path_size + 1, GFP_KERNEL);
	if (unlikely(evr == 0))
		return 0;

	evr->pid = current->pid;
	strcpy(evr->full_path, uuid);
	strcat(evr->full_path, path);
	evr->result = VERIFY_UNKNOWN;

	mutex_lock(&elf_ver_dev.lock);
	list_add_tail(&evr->list, &elf_ver_dev.requests);
	mutex_unlock(&elf_ver_dev.lock);
	pr_info("elf-to-be-verified: %s\n", evr->full_path);
	return evr;
}

static void clean_requests_locked(pid_t pid)
{
	struct list_head *p = NULL, *next = NULL;
	elf_verifier_req *req = NULL;

	mutex_lock(&elf_ver_dev.lock);
	list_for_each_safe(p, next, &elf_ver_dev.requests) {
		req = list_entry(p, elf_verifier_req, list);
		if (req->pid != pid)
			continue;

		list_del(p);
		kfree(req);
	}
	mutex_unlock(&elf_ver_dev.lock);
}

static int get_response_nolock(elf_verifier_req* evr)
{
	struct list_head *p = NULL, *next = NULL;
	elf_verifier_req *req = NULL;
	int ret = 0;

	list_for_each_safe(p, next, &elf_ver_dev.requests) {
		req = list_entry(p, elf_verifier_req, list);
		if (req->pid != evr->pid || strcmp(req->full_path, evr->full_path))
			continue;

		ret = req->result;
		list_del(p);
		kfree(req);
		return ret;
	}
	return VERIFY_UNKNOWN;
}

int verify_elf(const char* root, const char* path,
	       unsigned int major, unsigned int minor)
{
	DECLARE_WAITQUEUE(wait, current);
	elf_verifier_req *evr = NULL;
	int ret = 0;

	evr = create_evr(root, path, major, minor);
	if (evr == 0)
		return VERIFY_NOMEM;

	add_wait_queue(&elf_ver_dev.resp_wq, &wait);
	wake_up_interruptible(&elf_ver_dev.reqs_wq);

	ret = VERIFY_UNKNOWN;
	mutex_lock(&elf_ver_dev.lock);
	for (;;) {
		__set_current_state(TASK_INTERRUPTIBLE);
		mutex_unlock(&elf_ver_dev.lock);
		schedule();

		if (signal_pending(current)) {
			pr_info("[%s] clean all requests\n", __func__);
			clean_requests_locked(evr->pid);
			ret = VERIFY_INTR;
			goto out;
		}

		mutex_lock(&elf_ver_dev.lock);
		ret = get_response_nolock(evr);
		if (ret != VERIFY_UNKNOWN)
			break;
	}

	mutex_unlock(&elf_ver_dev.lock);
out:
	remove_wait_queue(&elf_ver_dev.resp_wq, &wait);
	set_current_state(TASK_RUNNING);
	return ret;
}

int register_elf_verifier_dev(void)
{
	int error = 0;

	if (_dev_init != 0) {
		return 0;
	}
	_dev_init = 1;

	elf_ver_dev.misc.minor = MISC_DYNAMIC_MINOR;
	elf_ver_dev.misc.name = DEV_NAME;
	elf_ver_dev.misc.fops = &misc_dev_fops;
	elf_ver_dev.controller = 0;
	mutex_init(&elf_ver_dev.lock);
	INIT_LIST_HEAD(&elf_ver_dev.requests);
	init_waitqueue_head(&elf_ver_dev.resp_wq);
	init_waitqueue_head(&elf_ver_dev.reqs_wq);
	error = misc_register(&elf_ver_dev.misc);
	if (!error)
		_dev_init = 2;
	else
		_dev_init = 0;
	return error;
}

void unregister_elf_verifier_dev(void)
{
	misc_deregister(&elf_ver_dev.misc);
}
