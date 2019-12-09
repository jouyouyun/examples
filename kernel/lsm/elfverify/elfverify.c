/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/lsm_hooks.h>
#include <linux/path.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <uapi/asm-generic/mman-common.h>

#include "partition.h"
#include "chdev.h"
#include "utils.h"

// only use consecutive letters and numbers, otherwise
// 'security_add_hooks' will fail
#define ELFVERIFY_LSM_NAME "elfverify"

#define MAX_COMM_LEN 16

extern int _dev_init;
static int _mount_init = 0;

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

	// init mountinfo and register device
	// systemd as the /sbin/init
	// TODO(jouyouyun): check command absolute path '/lib/systemd/systemd'
	if ((_dev_init == 0 || _mount_init == 0) && strcmp(cmd, "init") == 0) {
		/* pr_info("[%s] command: %s, device(%d), mount(%d)\n", */
		/* 	__func__, cmd, _dev_init, _mount_init); */
		error = register_elf_verifier_dev();
		if (!error)
			init_mounts_info();
		goto out;
	}

	if (_dev_init != 2 || _mount_init != 2) {
		goto out;
	}

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
	if (*root == '/' && is_whitelist_mountpoint(root))
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
	if (verify_elf(root, path, major, minor))
		error = -EACCES;

	pr_info("[%s] root: %s, cmd: %s, verify: %d\n", __func__, root, cmd, error);
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
	if (!is_whitelist_mountpoint(raw_path) || strcmp(raw_path, "/") == 0)
		return 0;

	if (!is_mnt_ns_valid()) {
		pr_warn("[%s] mnt ns invalid\n", __func__);
		goto out;
	}

	if (is_special_mp(raw_path)) {
		pr_warn("[%s] special mpunt point\n", __func__);
		goto out;
	}

	// Not yet mounted, read from sysfs
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

static struct security_hook_list hooked_list[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(mmap_file, hook_mmap_file),
	LSM_HOOK_INIT(sb_mount, hook_sb_mount),
};

static int __init elfverify_init(void)
{
	security_add_hooks(hooked_list, ARRAY_SIZE(hooked_list), ELFVERIFY_LSM_NAME);
	pr_info("LSM initialized: %s\n", ELFVERIFY_LSM_NAME);

	return 0;
}

security_initcall(elfverify_init);
