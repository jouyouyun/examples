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

#define ALLOC_UNIT (1<<12)

void get_root(char* root, unsigned int major, unsigned int minor);
void lsm_add_partition(const char* dir_name, int major, int minor);
void drop_partition(const char* dir_name, int major, int minor);
static char* read_file_content(const char* filename, int *real_size);
int is_special_mp(const char* mp);
int is_mnt_ns_valid(void);
int get_major_minor(const char* dir_name, unsigned int* major, unsigned int* minor);
int parse_mounts_info(char* buf);

typedef struct __ftrace_partition__ {
	unsigned int major, minor;
	struct list_head list;
	char root[0];
} ftrace_partition;

static DECLARE_RWSEM(rw_parts);
static LIST_HEAD(partitions);

#define VERIFY_OK		0
#define VERIFY_FAIL		1
#define VERIFY_NOMEM	2
#define VERIFY_INTR		3
#define VERIFY_UNKNOWN	4

#define DEV_NAME	"elf_verifier"

static int _dev_init = 0;
static int _mount_init = 0;

void get_root(char* root, unsigned int major, unsigned int minor)
{
	ftrace_partition *part = NULL;

	// TODO(jouyouyun): device has multi mount points
	*root = 0;
	down_read(&rw_parts);
	list_for_each_entry(part, &partitions, list) {
		if (part->major == major && part->minor == minor) {
			strcpy(root, part->root);
			break;
		}
	}
	up_read(&rw_parts);
}

void lsm_add_partition(const char* dir_name, int major, int minor)
{
	ftrace_partition *part = NULL;

	part = kzalloc(sizeof(ftrace_partition) + strlen(dir_name) + 1, GFP_KERNEL);
	if (unlikely(part == 0)) {
		pr_err("kzalloc failed and thus cant add %s [%d, %d] to partitions\n",
			dir_name, major, minor);
		return;
	}

	part->major = major;
	part->minor = minor;
	strcpy(part->root, dir_name);
	down_write(&rw_parts);
	list_add_tail(&part->list, &partitions);
	up_write(&rw_parts);
}

void drop_partition(const char* dir_name, int major, int minor)
{
	struct list_head *p = NULL, *next = NULL;
	ftrace_partition *part = NULL;

	down_write(&rw_parts);
	list_for_each_safe(p, next, &partitions) {
		part = list_entry(p, ftrace_partition, list);
		if (part->major != major || part->minor != minor ||
			strcmp(part->root, dir_name))
			continue;

		pr_info("partition %s [%d, %d] umounted\n", part->root, part->major, part->minor);
		list_del(p);
		kfree(part);
		break;
	}
	up_write(&rw_parts);
}

/*
  kernel_read_file_from_path cant be used here because:
  1. procfs doesnt have i_size, which is used by kernel_read_file, which is in turn called by k_r_f_f_p
  2. we want to avoid security_XXX calls
*/
static char *read_file_content(const char *filename, int *real_size)
{
	struct file *filp = NULL;
	char *buf = NULL;
	mm_segment_t old_fs;
	loff_t off = 0;
	int size = ALLOC_UNIT;

	if (unlikely(filename == NULL) || unlikely(real_size == NULL))
		return NULL;

	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		pr_warn("[%s] failed to open: %s\n", __func__, filename);
		return NULL;
	}

	old_fs = get_fs();
	set_fs(get_fs());

	while(1) {
		buf = kzalloc(size, GFP_KERNEL);
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
		if (*real_size > 0 && *real_size < size) {
			buf[*real_size] = 0;
			break;
		}

		kfree(buf);
		buf = NULL;
		if (*real_size != 0)
			size += ALLOC_UNIT;
		else
			break;
	}

	set_fs(old_fs);
	filp_close(filp, 0);
	return buf;
}

static int mounted_at(const char* mp, const char* root)
{
	return strcmp(mp, root) == 0 ||
		(strlen(mp) > strlen(root) && strstr(mp, root) == mp && mp[strlen(root)] == '/');
}

int is_special_mp(const char *mp)
{
	static char *mp_list[] = {"/sys", "/proc", "/run", "/dev", NULL};
	int i = 0;

	if (!mp || *mp != '/')
		return 1;

	for (; mp_list[i]; i++)
		if (mounted_at(mp, mp_list[i]))
			return 1;

	return 0;
}

int is_mnt_ns_valid(void)
{
	static struct mnt_namespace* init_mnt_ns = NULL;

	if (init_mnt_ns == 0) {
		if (current && current->nsproxy)
			init_mnt_ns = current->nsproxy->mnt_ns;
		return init_mnt_ns != NULL;
	}

	if (current && current->nsproxy && current->nsproxy->mnt_ns != init_mnt_ns)
		return 0;

	return 1;
}

int get_major_minor(const char* dir_name, unsigned int* major, unsigned int* minor)
{
	struct path path;

	if (kern_path(dir_name, LOOKUP_FOLLOW, &path))
		return 1;

	if (unlikely(path.dentry == NULL) || unlikely(path.dentry->d_sb == NULL))
		return 1;

	*major = MAJOR(path.dentry->d_sb->s_dev);
	*minor = MINOR(path.dentry->d_sb->s_dev);

	// null device
	if (*major == 3)
		return 1;

	path_put(&path);
	return 0;
}

static int get_major_minor_from_sysfs(const char *dev_name, unsigned int* major, unsigned int* minor)
{
	char *content = NULL;
	char filename[PATH_MAX + 21] = {0};
	int size = 0;
	unsigned int ma = 0, mi = 0;

	if (strstr(dev_name, "/dev/") != dev_name)
		return 1;

	memset(filename, 0, PATH_MAX + 21);
	sprintf(filename, "/sys/class/block/%s/dev", dev_name + 5);
	content = read_file_content(filename, &size);
	if (!content)
		return 1;

	if (content[size-1] == '\n')
		content[size-1] = '\0';
	if (sscanf(content, "%u:%u", &ma, &mi) != 2) {
		kfree(content);
		content = NULL;
		return 1;
	}

	kfree(content);
	content = NULL;

	if (ma == 0 || mi == 0)
		return 1;

	*major = ma;
	*minor = mi;
	return 0;
}

int parse_mounts_info(char* buf)
{
	ftrace_partition* part = NULL;
	char *mp = NULL;
	char *line = NULL;
	unsigned int major = 0, minor = 0;
	int parts_count = 0;

	mp = kzalloc(NAME_MAX, GFP_KERNEL);
	if (unlikely(mp == NULL))
		return -1;

	line = buf;
	while (sscanf(line, "%*d %*d %u:%u %*s %250s %*s %*s %*s %*s %*s %*s\n", &major, &minor, mp) == 3) {
		line = strchr(line, '\n') + 1;

		if (is_special_mp(mp)) {
			memset(mp, 0, NAME_MAX);
			continue;
		}

		part = kzalloc(sizeof(ftrace_partition) + strlen(mp) + 1, GFP_KERNEL);
		if (unlikely(part == 0)) {
			pr_err("ftrace-partition kzalloc failed for %s\n", mp);
			memset(mp, 0, NAME_MAX);
			continue;
		}
		part->major = major;
		part->minor = minor;
		strcpy(part->root, mp);
		memset(mp, 0, NAME_MAX);
		list_add_tail(&part->list, &partitions);
	}
	kfree(mp);
	mp = NULL;

	// __init section doesnt need lock
	part = NULL;
	list_for_each_entry(part, &partitions, list) {
		parts_count++;
		pr_info("mp: %s, major: %d, minor: %d\n", part->root, part->major, part->minor);
	}
	return parts_count;
}

static void init_mounts_info(void)
{
	char *buf = NULL;
	char *cur_path = NULL;
	char *path_buf = NULL;
	char *cmdline = NULL;
	int size = 0;
	int parts_count = 0;
	int i = 0;

	if (_mount_init != 0)
		return;
	_mount_init = 1;

	if (!is_mnt_ns_valid()) {
		_mount_init = 0;
		return;
	}

	buf = read_file_content("/proc/self/mountinfo", &size);
	if (!buf) {
		_mount_init = 0;
		return;
	}

	parts_count = parse_mounts_info(buf);
	if (parts_count == -1) {
		_mount_init = 0;
		return;
	}
	_mount_init = 2;
	kfree(buf);

	path_buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (likely(path_buf))
		cur_path = file_path(get_task_exe_file(current), path_buf, PATH_MAX);

	size = 0;
	cmdline = read_file_content("/proc/self/cmdline", &size);
	if (cmdline)
		for (; i < size; i++) {
			if (cmdline[i] != 0) continue;
			if (cmdline[i+1] == 0) break;
			cmdline[i] = ' ';
		}

	pr_info("partition count: %d, comm[%d]: %s, path: %s, cmdline: %s\n",
		parts_count, current->pid, current->comm?current->comm:"NULL",
		cur_path?cur_path:"NULL", cmdline);
	if (path_buf)
		kfree(path_buf);
	if (cmdline)
		kfree(cmdline);
}

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

static elf_verifier_req* create_evr(const char* root, const char* path)
{
	int path_size = 0;
	elf_verifier_req *evr = NULL;

	path_size = strlen(root) == 1 ? strlen(path) : strlen(root) + strlen(path);
	evr = kzalloc(sizeof(elf_verifier_req) + path_size + 1, GFP_KERNEL);
	if (unlikely(evr == 0))
		return 0;

	evr->pid = current->pid;
	if (strlen(root) > 1)
		strcpy(evr->full_path, root);
	else
		evr->full_path[0] = 0;

	strcat(evr->full_path, path);
	pr_info("elf-to-be-verified: %s\n", evr->full_path);
	evr->result = VERIFY_UNKNOWN;

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

int verify_elf(const char* root, const char* path)
{
	DECLARE_WAITQUEUE(wait, current);
	elf_verifier_req *evr = NULL;
	int ret = 0;

	//system partitions include /, /usr and /boot
	if (strcmp(root, "/") == 0 || strcmp(root, "/usr") == 0 || strcmp(root, "/boot") == 0)
		return VERIFY_OK;

	evr = create_evr(root, path);
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
	char *root = NULL;
	dev_t dev;
	unsigned int major = 0, minor = 0;
	int error = 0;

	if (unlikely(file == NULL) || unlikely(current == NULL) ||
	    unlikely(current->comm == NULL))
		return 0;

	if (prot != (PROT_READ | PROT_EXEC) || (flags & MAP_PRIVATE) == 0)
		return 0;

	cmd = kzalloc(MAX_COMM_LEN + 1, GFP_KERNEL);
	if (unlikely(cmd == NULL))
		return 0;

	if (likely(current && current->comm))
		strcpy(cmd, current->comm);

	/* if (_dev_init == 2 && _mount_init == 2) { */
	/* 	pr_info("[%s] has inited, cmd: %s\n", __func__, cmd); */
	/* 	kfree(cmd); */
	/* 	return 0; */
	/* } */

	// init mountinfo and register device
	// systemd as the /sbin/init
	// TODO(jouyouyun): check command absolute path '/lib/systemd/systemd'
	if ((_dev_init == 0 || _mount_init == 0) && strcmp(cmd, "systemd") == 0) {
		pr_info("[%s] command: %s, device(%d), mount(%d)\n",
			__func__, cmd, _dev_init, _mount_init);
		error = register_elf_verifier_dev();
		if (!error)
			init_mounts_info();
		goto out;
	}

	/* if (strstr(cmd, "lssssl") == 0) { */
	/* 	goto out; */
	/* } */

	if (unlikely(file->f_path.dentry == NULL) ||
	    unlikely(file->f_path.dentry->d_sb == NULL) ||
	    unlikely(file->f_path.dentry->d_name.name == NULL))
		goto out;

	root = kzalloc(NAME_MAX, GFP_KERNEL);
	if (unlikely(root == NULL))
		goto out;

	dev = file->f_path.dentry->d_sb->s_dev;
	major = MAJOR(dev);
	minor = MINOR(dev);
	get_root(root, major, minor);
	if (*root == 0)
		goto out;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL))
		goto out;

	path = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	if (IS_ERR(path)) {
		goto out;
	}

	/* pr_info("current[%d]: %s, root: %s, mmap-file: %s, flags: 0x%lX\n", */
	/* 	current->pid, cmd, root, path, flags); */
	error = 0;
	if (verify_elf(root, path))
		error = -EACCES;

out:
	if (buf) {
		kfree(buf);
		buf = NULL;
	}
	if (root) {
		kfree(root);
		root = NULL;
	}

	kfree(cmd);
	cmd = NULL;
	return error;
}

static int hook_sb_mount(const char *dev_name, const struct path *path,
		const char *type, unsigned long flags, void *data)
{
	char *root = NULL;
	char *buf = NULL;
	char *raw_path = NULL;
	unsigned int major = 0, minor = 0;

	if (_mount_init != 2)
		return 0;

	if (unlikely(dev_name == NULL) || unlikely(path == NULL) || unlikely(path->dentry == NULL) ||
	    unlikely(type == NULL))
		return 0;

	buf = kzalloc(PATH_MAX + 1, GFP_KERNEL);
	if (unlikely(buf == NULL))
		return 0;

	raw_path = dentry_path_raw(path->dentry, buf, PATH_MAX);
	if (IS_ERR(raw_path))
		goto out;

	pr_info("[%s] hook mount, device: %s, type: %s, raw name: %s\n", __func__, dev_name, type, raw_path);
	if (!is_mnt_ns_valid()) {
		pr_warn("[%s] mnt ns invalid\n", __func__);
		goto out;
	}

	if (is_special_mp(raw_path)) {
		pr_warn("[%s] special mpunt point\n", __func__);
		goto out;
	}

	// Not yet mounted, replace dir_path with dev_name
	if (get_major_minor_from_sysfs(dev_name, &major, &minor)) {
	/* if (get_major_minor(dev_name, &major, &minor)) { */
		pr_warn("[%s] failed to get_major_minor\n", __func__);
		goto out;
	}
	pr_info("[%s] get major for device: %s, major: %u, minor: %u\n", __func__, dev_name, major, minor);

	root = kzalloc(NAME_MAX + 1, GFP_KERNEL);
	if (unlikely(root == NULL))
		goto out;

	get_root(root, major, minor);
	pr_info("[%s] get root: %s, major: %u, minor: %u\n", __func__, root, major, minor);
	// TODO(jouyouyun): handle mount failure
	if (*root == 0 || strcmp(root, raw_path))
		lsm_add_partition(raw_path, major, minor);

out:
	if (buf)
		kfree(buf);
	if (root)
		kfree(root);
	return 0;
}

static int hook_sb_umount(struct vfsmount *mnt, int flags)
{
	char *root = NULL;
	dev_t dev;
	unsigned int major = 0, minor = 0;

	if (_mount_init != 2)
		return 0;

	if (unlikely(mnt == NULL) || unlikely(mnt->mnt_sb == NULL))
		return 0;

	root = kzalloc(NAME_MAX, GFP_KERNEL);
	if (unlikely(root == NULL))
		return 0;

	dev = mnt->mnt_sb->s_dev;
	major = MAJOR(dev);
	minor = MINOR(dev);
	get_root(root, major, minor);
	pr_info("[%s] root: %s, major: %u, minor: %u\n", __func__, root, major, minor);
	if (*root == 0)
		goto out;

	// TODO(jouyouyun): handle umount failure
	drop_partition(root, major, minor);
out:
	if (root) {
		kfree(root);
		root = NULL;
	}

	return 0;
}

static struct security_hook_list hooked_list[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(mmap_file, hook_mmap_file),
	LSM_HOOK_INIT(sb_mount, hook_sb_mount),
	LSM_HOOK_INIT(sb_umount, hook_sb_umount),
};

static int __init monitor_init(void)
{
	security_add_hooks(hooked_list, ARRAY_SIZE(hooked_list), MONITOR_LSM_NAME);
	pr_info("LSM initialized: %s\n", MONITOR_LSM_NAME);

	return 0;
}

security_initcall(monitor_init);
