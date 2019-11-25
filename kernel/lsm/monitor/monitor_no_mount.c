/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * hook_monitor.c -- test LSM under 4.19
 *
 * Written on 星期五,  8 十一月 2019.
 */

#include <linux/lsm_hooks.h>
#include <linux/sysctl.h>
#include <linux/binfmts.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/version.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/list.h>
#include <linux/limits.h>
#include <linux/nsproxy.h>
#include <linux/namei.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif
#include <linux/miscdevice.h>
#include <uapi/asm-generic/mman-common.h>

// only use consecutive letters and numbers, otherwise
// 'security_add_hooks' will fail
#define MONITOR_LSM_NAME "monitor"

#define DEV_NAME	"elf_verifier"

#define USERSPACE_VERIFY_FILE "/run/deepin_elf_verify.pid"

#define ALLOC_UNIT (1<<12)

#define VERIFY_OK		0
#define VERIFY_FAIL		1
#define VERIFY_NOMEM	2
#define VERIFY_INTR		3
#define VERIFY_UNKNOWN	4

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

static char* read_file_content(const char* filename, size_t *real_size);

static int _dev_init = 0;

/*
  kernel_read_file_from_path cant be used here because:
  1. procfs doesnt have i_size, which is used by kernel_read_file, which is in turn called by k_r_f_f_p
  2. we want to avoid security_XXX calls
*/
static char *read_file_content(const char *filename, size_t *real_size)
{
	struct file *filp = NULL;
	char *buf = NULL;
	mm_segment_t old_fs;
	loff_t off = 0;
	size_t size = ALLOC_UNIT;

	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		/* pr_err("[%s] failed to open: %s\n", __func__, filename); */
		return NULL;
	}

	old_fs = get_fs();
	set_fs(get_fs());

	while(1) {
		buf = kmalloc(size, GFP_KERNEL);
		if (unlikely(buf == NULL))
			break;

		off = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
		*real_size = kernel_read(filp, buf, size, &off);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
		*real_size = vfs_read(filp, (char __user*)buf, size, &off);
#else
		*real_size = __vfs_read(filp, (char __user*)buf, size, &off);
#endif
		pr_info("[%s] read size: real(%ld), size: %ld\n", __func__, *real_size, size);
		if (*real_size > 0 && *real_size < size) {
			buf[*real_size] = 0;
			break;
		}

		kfree(buf);
		if (*real_size != 0)
			size += ALLOC_UNIT;
		else
			break;
	}

	pr_info("[%s] %s size: %ld\n", __func__, filename, *real_size);
	set_fs(old_fs);
	filp_close(filp, 0);
	return buf;
}

static int has_userspace_verify(void)
{
	char *buf = NULL;
	size_t size = 0;

	buf = read_file_content(USERSPACE_VERIFY_FILE, &size);
	if (!buf)
		return 0;

	return 1;
}

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

	kbuf = kmalloc(size, GFP_KERNEL);
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

static elf_verifier_req* create_evr(const char* path)
{
	int path_size = 0;
	elf_verifier_req *evr = NULL;

	path_size = strlen(path);
	evr = kmalloc(sizeof(elf_verifier_req) + path_size + 1, GFP_KERNEL);
	if (unlikely(evr == 0))
		return 0;

	evr->pid = current->pid;
	evr->full_path[0] = 0;

	strcat(evr->full_path, path);
	evr->result = VERIFY_UNKNOWN;
	pr_info("elf-to-be-verified: %s\n", evr->full_path);

	mutex_lock(&elf_ver_dev.lock);
	list_add_tail(&evr->list, &elf_ver_dev.requests);
	mutex_unlock(&elf_ver_dev.lock);
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

int verify_elf(const char* path)
{
	DECLARE_WAITQUEUE(wait, current);
	elf_verifier_req *evr = NULL;
	int ret = 0;

	evr = create_evr(path);
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

#define MAX_COMM_LEN 16

static int hook_mmap_file(struct file *file, unsigned long reqprot,
			  unsigned long prot, unsigned long flags)
{
	char *path = NULL;
	char *buf = NULL;
	char *cmd = NULL;
	int error = 0;

	if (unlikely(current == NULL) || unlikely(file== NULL))
		return 0;

	if (prot != (PROT_READ | PROT_EXEC) || (flags & MAP_PRIVATE) == 0)
		return 0;

	cmd = kmalloc(MAX_COMM_LEN + 1, GFP_KERNEL);
	if (unlikely(cmd == NULL))
		return 0;

	cmd[MAX_COMM_LEN] = '\0';
	if (likely(current->comm))
		strcpy(cmd, current->comm);

	// init mountinfo and register device
	// systemd as the /sbin/init
	// TODO(jouyouyun): check command absolute path '/lib/systemd/systemd'
	if (_dev_init == 0 && strcmp(cmd, "systemd") == 0) {
		pr_info("[%s] command: %s, device(%d)\n", __func__, cmd, _dev_init);
		error = register_elf_verifier_dev();
		if (error)
			pr_warn("[%s] failed to register elf verify device: %d\n", __func__, error);
		error = 0;
		goto out;
	}

	// check userspace elf verify whether exists
	// TODO(jouyouyun): check file root(mount point) in this module
	if (has_userspace_verify() == 0) {
		goto out;
	}

	if (unlikely(file->f_path.dentry == NULL))
		goto out;

	buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL))
		goto out;

	path = dentry_path_raw(file->f_path.dentry, buf, PATH_MAX);
	if (IS_ERR(path)) {
		kfree(buf);
		error = 0;
		goto out;
	}

	pr_info("current[%d]: %s, mmap-file: %s, flags: 0x%lX\n", current->pid, cmd, path, flags);

	error = 0;
	if (verify_elf(path))
		error = -EACCES;

	kfree(buf);
	pr_info("[%s] elf verify %s[%d]: %d\n", __func__, cmd, current->pid, error);

out:
	if (cmd) {
		kfree(cmd);
	}
	return error;
}

static struct security_hook_list hooked_list[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(mmap_file, hook_mmap_file),
};

static int __init monitor_init(void)
{
	security_add_hooks(hooked_list, ARRAY_SIZE(hooked_list), MONITOR_LSM_NAME);
	pr_info("LSM initialized: %s\n", MONITOR_LSM_NAME);

	return 0;
}

security_initcall(monitor_init);
