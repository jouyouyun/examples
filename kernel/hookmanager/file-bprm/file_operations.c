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

#include <linux/binfmts.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/lsm_uos_hook_manager.h>
#include <linux/module.h>
#include <linux/slab.h>

#define MOD_NAME "file_operations"

struct hook_wrapper {
  int id;
  struct uos_hook_cb_entry cb;
};

static void dump_file(struct file *file, const char *msg)
{
  char *buf = NULL;
  char *tmp = NULL;
  char *dbuf = NULL, *dtmp = NULL;
  unsigned long ino = 0;

  buf = kzalloc(PATH_MAX, GFP_KERNEL);
  dbuf = kzalloc(PATH_MAX, GFP_KERNEL);
  if (unlikely(buf == NULL) || unlikely(dbuf == NULL)) {
    printk("[%s] (%s) failed to alloc", MOD_NAME, msg);
    goto out;
  }

  // tmp = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
  tmp = d_path((const struct path *)&(file->f_path), buf, PATH_MAX);
  dtmp = dentry_path_raw(file->f_path.dentry, dbuf, PATH_MAX);
  if (IS_ERR(tmp) || IS_ERR(dtmp)) {
    printk("[%s] (%s) failed to get path", MOD_NAME, msg);
    goto free;
  }

  if (strstr(tmp, "TEST") != NULL || strstr(dtmp, "TEST") != NULL) {
    ino = (file->f_inode) ? file->f_inode->i_ino : 0;
    printk("[%s] (%s) , comm(%d-%d:%s), path: %s, dentry: %s, inode: %lu",
           MOD_NAME, msg, current->pid, task_ppid_nr(current),
           current->comm ? current->comm : "NULL", tmp, dtmp, ino);
  }

free:
  kfree(buf);
  buf = NULL;
  kfree(dbuf);
  dbuf = NULL;
out:
  return ;	
}

static int hook_file_open(struct file *file) {
	dump_file(file, "file_open");
	return 0;
}

static void dump_bprm(struct linux_binprm *bprm, const char *msg) {
	// 5.10
	/* if (bprm->executable) */
	/* 	retval = dump_file(bprm->executable, msg); */
	/* if (bprm->interpreter) */
	/* 	retval = dump_file(bprm->interpreter, msg); */
        if (bprm->file)
		dump_file(bprm->file, msg);

	if (strstr(bprm->filename,"TEST") != NULL)
		printk("\tfilename (%s)", bprm->filename?bprm->filename:"NULL");
	if (strstr(bprm->interp, "TEST") != NULL)
		printk("\tinterp(%s)", bprm->interp ? bprm->interp : "NULL");
        // 5.10
        /* printk("\tfilename (%s), \n\tinterp(%s), \n\tfdpath(%s)", */
        /*        bprm->filename ? bprm->filename : "NULL", */
        /*        bprm->interp ? bprm->interp : "NULL", */
        /*        bprm->fdpath ? bprm->fdpath : "NULL"); */
}

static int hook_bprm_committed_creds(struct linux_binprm *bprm) {
	dump_bprm(bprm, "bprm_committed_creds");
	return 0;
}

static int hook_bprm_check_security(struct linux_binprm *bprm) {
	dump_bprm(bprm, "bprm_check_security");
	return 0;
}

static struct hook_wrapper entries[] = {
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
        .id = UOS_BPRM_COMMITTED_CREDS,
        .cb =
            {
                .owner = MOD_NAME,
                .cb_addr = (unsigned long)hook_bprm_committed_creds,
                .ret_type = UOS_HOOK_RET_TY_NONE,
                .arg_len = 1,
            },
    },
    {
        .id = UOS_BPRM_CHECK_SECURITY,
        .cb =
            {
                .owner = MOD_NAME,
                .cb_addr = (unsigned long)hook_bprm_check_security,
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
      printk("[%s] failed to cancel hook %d, error: %d", MOD_NAME,
             entries[i].id, err);
  }
}

int __init mod_init(void) {
  int i = 0;
  int err = 0;

  for (; entries[i].id != UOS_HOOK_NONE; i++) {
    err = uos_hook_register(entries[i].id, &entries[i].cb);
    if (err) {
      printk("[%s] failed to register hook: %d, err: %d", MOD_NAME,
             entries[i].id, err);
      break;
    }
  }

  if (err) {
    unregister_hook(i);
    return -1;
  }

  return 0;
}

void __exit mod_exit(void) { unregister_hook(-1); }

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("Test file operation hook");
