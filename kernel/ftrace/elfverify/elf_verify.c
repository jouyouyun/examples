/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/namei.h>
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

#define HOOK(_name, _function, _original) {	\
		.name = (_name),		\
		.function = (_function),	\
		.original = (_original),	\
	}

struct elf_hook {
	const char *name;
	unsigned long function;
	unsigned long original;
	struct ftrace_ops fops;
};

static int hook_mmap_file(struct file *file, unsigned long prot, unsigned long flags);
static long hook_do_mount(const char *dev_name, const char __user *name,
			  const char *type, unsigned long flags, void *data);
static struct elf_hook* get_ftrace_op(const char* name);

#define ELF_HOOK_GETOP(name) static struct elf_hook* fop = 0;	\
	if (fop == 0)							\
		fop = get_ftrace_op(name);				\


extern int _dev_init;
static int _mount_init = 0;

static struct elf_hook hooked_funcs[] = {
	HOOK("security_mmap_file", (unsigned long)&hook_mmap_file, 0),
	HOOK("do_mount", (unsigned long)&hook_do_mount, 0),
	HOOK(NULL, 0, 0),
};

static struct elf_hook* get_ftrace_op(const char* name)
{
	for (int i = 0; hooked_funcs[i].name; i++) {
		if (strcmp(name, hooked_funcs[i].name) == 0)
			return &hooked_funcs[i];
	}
	return 0;
}

static char *get_absolute_path(const struct file *file)
{
	struct path path;
	char *buf = NULL;
	char *tmp = NULL;
	char *filepath = NULL;
	int err = 0;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL))
		return NULL;

	tmp = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	if (IS_ERR(tmp)) {
		goto error;
	}

	if (!d_is_symlink(file->f_path.dentry)) {
		goto ret;
	}

	err = kern_path(tmp, LOOKUP_FOLLOW, &path);
	if (err || unlikely(path.dentry == NULL)) {
		goto error;
	}

	memset(buf, 0, PATH_MAX);
	tmp = d_absolute_path((const struct path *)&path, buf, PATH_MAX);
	if (IS_ERR(tmp)) {
		goto error;
	}

ret:
	filepath = kzalloc(PATH_MAX, GFP_KERNEL);
	if (likely(filepath)) {
		strcpy(filepath, tmp);
	}

error:
	kfree(buf);
	buf = NULL;
	return filepath;
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

static int hook_mmap_file(struct file *file, unsigned long prot, unsigned long flags)
{
	char *path = NULL;
	char *cmd = NULL;
	char *root = NULL;
	dev_t dev;
	unsigned int major = 0, minor = 0;
	int error = 0;
	static int (*old_fn)(struct file *, unsigned long, unsigned long) = NULL;
	ELF_HOOK_GETOP("security_mmap_file");

	*((unsigned long *)&old_fn) = fop->original + MCOUNT_INSN_SIZE;
	error = old_fn(file, prot, flags);

	if (error || unlikely(file == NULL) || unlikely(current == NULL) ||
	    unlikely(current->comm == NULL))
		return 0;

	if (prot != (PROT_READ | PROT_EXEC) || (flags & MAP_PRIVATE) == 0)
		return 0;

	cmd = kzalloc(MAX_COMM_LEN + 1, GFP_KERNEL);
	if (unlikely(cmd == NULL))
		return 0;

	if (likely(current && current->comm))
		strcpy(cmd, current->comm);

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

	path = get_absolute_path(file);
	if (unlikely(path == NULL))
		goto out;

	pr_info("current[%d]: %s, root: %s, mmap-file: %s, flags: 0x%lX\n",
		current->pid, cmd, root, path, flags);
	error = 0;
	if (verify_elf(root, path, major, minor))
		error = -EACCES;

	pr_info("[%s] root: %s, cmd: %s, verify: %d\n", __func__, root, cmd, error);
out:
	if (path) {
		kfree(path);
		path = NULL;
	}
	if (root) {
		kfree(root);
		root = NULL;
	}

	kfree(cmd);
	cmd = NULL;
	return error;
}

static long hook_do_mount(const char *dev_name, const char __user *name,
			  const char *type, unsigned long flags, void *data)
{
	char *root = NULL;
	char *buf = NULL;
	unsigned int major = 0, minor = 0;
	int error = 0;
	static int (*old_fn)(const char *, const char __user *, const char *, unsigned long, void *) = 0;	ELF_HOOK_GETOP("do_mount");

	if (unlikely(dev_name == NULL) || unlikely(name == NULL) || unlikely(type == NULL))
		return 0;

	*((unsigned long *)&old_fn) = fop->original + MCOUNT_INSN_SIZE;
	error = old_fn(dev_name, name, type, flags, data);
	if (error || !is_mnt_ns_valid())
		return error;

	buf = kzalloc(NAME_MAX + 1, GFP_KERNEL);
	if (unlikely(buf == NULL))
		return 0;

	if (unlikely(strncpy_from_user(buf, name, NAME_MAX) < 0))
		return 0;

	pr_info("[%s] hook mount, device: %s, type: %s, name: %s\n", __func__, dev_name, type, buf);
	if (!is_whitelist_mountpoint(buf) || strcmp(buf, "/") == 0)
		return 0;

	if (is_special_mp(buf)) {
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
	if (*root == 0 || strcmp(root, buf))
		lsm_add_partition(buf, major, minor);

out:
	if (buf)
		kfree(buf);
	if (root)
		kfree(root);
	return 0;
}

static void notrace ftrace_elf_fn(unsigned long ip, unsigned long parent_ip,
				  struct ftrace_ops *ops, struct pt_regs *regs)
{
	struct elf_hook *hook = NULL;

	if (within_module(parent_ip, THIS_MODULE)) {
		pr_info("[Debug] [%s] 0x%lx in this module\n", __func__, parent_ip);
		return;
	}

	hook = container_of(ops, struct elf_hook, fops);
	regs->ip = hook->function;
}

static void remove_ftrace_hooks(int end)
{
	for (int i = 0; i <= end && hooked_funcs[i].name; i++) {
		unregister_ftrace_function(&hooked_funcs[i].fops);
		ftrace_set_filter_ip(&hooked_funcs[i].fops, hooked_funcs[i].original, 1, 0);
	}
}

static int install_ftrace_hooks(void)
{
	for (int i = 0; hooked_funcs[i].name; i++) {
		hooked_funcs[i].original = kallsyms_lookup_name(hooked_funcs[i].name);
		if (hooked_funcs[i].original == 0) {
			pr_err("kallsyms-lookup-name %s failed\n", hooked_funcs[i].name);
			return -1;
		}
		hooked_funcs[i].fops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY | FTRACE_OPS_FL_RECURSION_SAFE;
		hooked_funcs[i].fops.func = ftrace_elf_fn;
		int ret = ftrace_set_filter_ip(&hooked_funcs[i].fops, hooked_funcs[i].original, 0, 0);
		if (ret) {
			pr_err("%s ftrace-set-filter-ip err: %d\n", hooked_funcs[i].name, ret);
			remove_ftrace_hooks(i-1);
			return -2;
		}

		ret = register_ftrace_function(&hooked_funcs[i].fops);
		if (ret) {
			pr_err("%s reg-ftrace-func err: %d\n", hooked_funcs[i].name, ret);
			ftrace_set_filter_ip(&hooked_funcs[i].fops, hooked_funcs[i].original, 1, 0);
			remove_ftrace_hooks(i-1);
			return -3;
		}
	}
	return 0;
}

int __init init_module(void)
{
	int error = 0;

	init_mounts_info();

	error = install_ftrace_hooks();
	if (error)
		return error;

	error = register_elf_verifier_dev();
	if (!error) {
		remove_ftrace_hooks(3);
		return error;
	}

	pr_info("ELFVerify initialized: %s\n", ELFVERIFY_LSM_NAME);
	return 0;
}

void __exit cleanup_module(void)
{
	unregister_elf_verifier_dev();
	remove_ftrace_hooks(3);
	pr_info("ELFVerify exited: %s\n", ELFVERIFY_LSM_NAME);
}

MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("ELF guard implemented by ftrace");
MODULE_LICENSE("GPL");
