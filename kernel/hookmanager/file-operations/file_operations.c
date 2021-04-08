/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * file_remove.c -- file remove hook examples
 *
 * Written on 星期一,  4 一月 2021.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/lsm_uos_hook_manager.h>

#define MOD_NAME "file_operations"

struct rm_hook_entry {
	int id;
	struct uos_hook_cb_entry cb;
};

#ifdef CONFIG_SECURITY_PATH
static int hook_path_unlink(const struct path *dir, struct dentry *dentry)
{
	char *pbuf = NULL;
	char *ptmp = NULL;
	char *dbuf = NULL;
	char *dtmp = NULL;

	if ((strstr(current->comm, "rm") != NULL && strlen(current->comm) == 2) ||
	    (strstr(current->comm, "vim") != NULL && strlen(current->comm) == 3)) {
	} else {
		return 0;
	}

	printk("[%s] (%s) current: %s", MOD_NAME, __func__, current->comm);
	pbuf = kzalloc(PATH_MAX, GFP_KERNEL);
        dbuf = kzalloc(PATH_MAX, GFP_KERNEL);
        if (unlikely(pbuf == NULL) || unlikely(dbuf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
        }

        //ptmp = d_absolute_path(dir, pbuf, PATH_MAX);
        ptmp = d_path(dir, pbuf, PATH_MAX);
	dtmp = dentry_path_raw(dentry, dbuf, PATH_MAX);
	if (IS_ERR(ptmp) || IS_ERR(dtmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
	}

	printk("[%s] (%s) path: %s, dentry: %s", MOD_NAME, __func__, ptmp, dtmp);
free:
	kfree(pbuf);
	pbuf = NULL;
	kfree(dbuf);
	dbuf = NULL;
out:
	return 0;
}

static int hook_path_rmdir(const struct path *dir, struct dentry *dentry)
{
	char *pbuf = NULL;
	char *ptmp = NULL;
	char *dbuf = NULL;
	char *dtmp = NULL;

	if ((strstr(current->comm, "rm") != NULL && strlen(current->comm) == 2) ||
	    (strstr(current->comm, "vim") != NULL && strlen(current->comm) == 3)) {
	} else {
		return 0;
	}

	printk("[%s] (%s) current: %s", MOD_NAME, __func__, current->comm);
	pbuf = kzalloc(PATH_MAX, GFP_KERNEL);
	dbuf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(pbuf == NULL) || unlikely(dbuf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
	}

	//ptmp = d_absolute_path(dir, pbuf, PATH_MAX);
	ptmp = d_path(dir, pbuf, PATH_MAX);
	dtmp = dentry_path_raw(dentry, dbuf, PATH_MAX);
	if (IS_ERR(ptmp) || IS_ERR(dtmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
	}

	printk("[%s] (%s) path: %s, dentry: %s", MOD_NAME, __func__, ptmp, dtmp);
free:
	kfree(pbuf);
	pbuf = NULL;
	kfree(dbuf);
	dbuf = NULL;
out:
	return 0;
}
#endif

static int hook_inode_unlink(struct inode *dir, struct dentry *dentry)
{
	char *buf = NULL;
	char *tmp = NULL;

	if ((strstr(current->comm, "rm") != NULL && strlen(current->comm) == 2) ||
	    (strstr(current->comm, "vim") != NULL && strlen(current->comm) == 3)) {
	} else {
		return 0;
	}

	printk("[%s] (%s) current: %s", MOD_NAME, __func__, current->comm);
	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
	}

	tmp = dentry_path_raw(dentry, buf, PATH_MAX);
	if (IS_ERR(tmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
	}

	printk("[%s] (%s) dentry: %s", MOD_NAME, __func__, tmp);
free:
	kfree(buf);
	buf = NULL;
out:
	return 0;
}

static int hook_inode_rmdir(struct inode *dir, struct dentry *dentry)
{
	char *buf = NULL;
	char *tmp = NULL;

	if ((strstr(current->comm, "rm") != NULL && strlen(current->comm) == 2) ||
	    (strstr(current->comm, "vim") != NULL && strlen(current->comm) == 3)) {
	} else {
		return 0;
	}

	printk("[%s] (%s) current: %s", MOD_NAME, __func__, current->comm);
	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
	}

	tmp = dentry_path_raw(dentry, buf, PATH_MAX);
	if (IS_ERR(tmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
	}

	printk("[%s] (%s) dentry: %s", MOD_NAME, __func__, tmp);
free:
	kfree(buf);
	buf = NULL;
out:
	return 0;
}

static int hook_file_ioctl(struct file *file, unsigned int cmd,
                           unsigned long arg)
{
	char *buf = NULL;
	char *tmp = NULL;
	char *dbuf = NULL, *dtmp = NULL;

	if ((strstr(current->comm, "rm") != NULL && strlen(current->comm) == 2) ||
	    (strstr(current->comm, "vim") != NULL && strlen(current->comm) == 3)) {
	} else {
		return 0;
	}
	
	printk("[%s] (%s) current: %s", MOD_NAME, __func__, current->comm);
	buf = kzalloc(PATH_MAX, GFP_KERNEL);
        dbuf = kzalloc(PATH_MAX, GFP_KERNEL);
        if (unlikely(buf == NULL) || unlikely(dbuf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
	}
	
        //tmp = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
        tmp = d_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	dtmp = dentry_path_raw(file->f_path.dentry, dbuf, PATH_MAX);
	if (IS_ERR(tmp) || IS_ERR(dtmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
        }

        printk("[%s] (%s) path: %s, dentry: %s, cmd: %u, arg: %lu", MOD_NAME, __func__,
	       tmp, dtmp, cmd, arg);

free:
	kfree(buf);
	buf = NULL;
out:
        return 0;
}

static int hook_file_open(struct file *file)
{
	char *buf = NULL;
	char *tmp = NULL;
	char *dbuf = NULL, *dtmp = NULL;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	dbuf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL) || unlikely(dbuf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
	}

	// tmp = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	tmp = d_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	dtmp = dentry_path_raw(file->f_path.dentry, dbuf, PATH_MAX);
	if (IS_ERR(tmp) || IS_ERR(dtmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
	}

	if (strstr(tmp, "test_o") != NULL || strstr(dtmp, "test_o") != NULL) {
		printk("[%s] (%s) path: %s, dentry: %s", MOD_NAME, __func__, tmp, dtmp);
	}

free:
	kfree(buf);
	buf = NULL;
	kfree(dbuf);
	dbuf = NULL;
out:
	return 0;
}

static int hook_file_permission(struct file *file, int mask)
{
	char *buf = NULL;
	char *tmp = NULL;
        char *dbuf = NULL, *dtmp = NULL;

	if ((strstr(current->comm, "rm") != NULL && strlen(current->comm) == 2) ||
	    (strstr(current->comm, "vim") != NULL && strlen(current->comm) == 3)) {
	} else {
		return 0;
	}

        printk("[%s] (%s) current: %s", MOD_NAME, __func__, current->comm);
	buf = kzalloc(PATH_MAX, GFP_KERNEL);
        dbuf = kzalloc(PATH_MAX, GFP_KERNEL);
        if (unlikely(buf == NULL) || unlikely(dbuf == NULL)) {
		printk("[%s] (%s) failed to alloc", MOD_NAME, __func__);
		goto out;
        }

        // tmp = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	tmp = d_path((const struct path *)&(file->f_path), buf, PATH_MAX);
        dtmp = dentry_path_raw(file->f_path.dentry, dbuf, PATH_MAX);
        if (IS_ERR(tmp) || IS_ERR(dtmp)) {
		printk("[%s] (%s) failed to get path", MOD_NAME, __func__);
		goto free;
        }

        printk("[%s] (%s) path: %s, dentry: %s, mask: 0x%x", MOD_NAME, __func__, tmp, dtmp, mask);

free:
	kfree(buf);
	buf = NULL;
	kfree(dbuf);
	dbuf = NULL;
out:
	return 0;
}

static struct rm_hook_entry entries[] = {
#ifdef CONFIG_SECURITY_PATH
	{
		.id = UOS_PATH_UNLINK,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_path_unlink,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 2,
		},
	},
	{
		.id = UOS_PATH_RMDIR,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_path_rmdir,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 2,
		},
	},
#else
	{
		.id = UOS_INODE_UNLINK,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_inode_unlink,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 2,
		},
	},
	{
		.id = UOS_INODE_RMDIR,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_inode_rmdir,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 2,
		},
	},
#endif
	{
		.id = UOS_FILE_IOCTL,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_file_ioctl,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 3,
		},
	},
	{
		.id = UOS_FILE_PERMISSION,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_file_permission,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 2,
		},
	},
	{
		.id = UOS_FILE_OPEN,
		.cb =
		{
			.owner = MOD_NAME,
			.cb_addr = (unsigned long)hook_file_open,
			.ret_type = UOS_HOOK_RET_TY_INT,
			.arg_len = 1,
		},
	},
	{
		.id = UOS_HOOK_NONE,
	},
};

static void unregister_hook(int idx) {
	int i = 0;
	int err = 0;

	for (; entries[i].id != UOS_HOOK_NONE; i++) {
		if (idx != -1 && i >= idx)
			break;

		err = uos_hook_cancel(entries[i].id, entries[i].cb.owner);
		if (err)
			printk("[%s] failed to cancel hook %d, error: %d", MOD_NAME, entries[i].id, err);
	}
}

int __init mod_init(void)
{
	int i = 0;
	int err = 0;

	for (; entries[i].id != UOS_HOOK_NONE; i++) {
		err = uos_hook_register(entries[i].id, &entries[i].cb);
		if (err) {
			printk("[%s] failed to register hook: %d, err: %d", MOD_NAME, entries[i].id, err);
			break;
		}
	}

	if (err) {
		unregister_hook(i);
		return -1;
	}
	
	return 0;
}

void __exit mod_exit(void)
{
	unregister_hook(-1);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("Test file remove hook");
