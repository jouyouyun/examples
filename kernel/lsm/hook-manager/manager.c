/**
 * Copyright (C) 2020 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * manager.c -- The manager of the hook
 *
 * Written on 星期五, 26 六月 2020.
 */

#include <linux/lsm_hook.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/buffer_head.h>
#include <linux/net.h>
#include <linux/socket.h>

#include "hook.h"

// only use consecutive letters and numbers, otherwise
// 'security_add_hooks' will fail
#define UOS_HOOK_MANAGER_LSM_NAME "uosmanager"

static struct security_hook_list hooked_list[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(mmap_file, hook_mmap_file),
	LSM_HOOK_INIT(socket_create, hook_socket_create),
	LSM_HOOK_INIT(socket_listen, hook_socket_listen),
};

#define EXEC_HOOK_CB(hook, cb, ret) \
	{											  \
		struct list_head *p = NULL, *next = NULL; \
		hook_entry *entry = NULL;						  \
		hook_entry_list *entries = &hooked_entries[hook]; \
		mutex_lock(&entries->lock);						  \
		list_for_each_safe(p, next, &entries->entries) {  \
			entry = list_entry(p, hook_entry, list);	  \
			ret = (cb);									  \
			if (ret != 0)								  \
				break;									  \
		}												  \
		mutex_unlock(&entries->lock);					  \
	}

static void init_hook_entries(void)
{
	int i = 0;
	int len = HOOK_SOCKET_LISTEN+1;

	for (; i < len; i++) {
		mutex_init(&hooked_entries[i].lock);
		INIT_LIST_HEAD(&hooked_entries[i].entries);
	}
}

static int hook_mmap_file(struct file *file, unsigned long reqprot,
						  unsigned long prot, unsigned long flags)
{
	int ret = 0;
	// TODO(jouyouyun): check cb addr validity
	EXEC_HOOK_CB(HOOK_MMAP_FILE,
				 ((int(*)(void))entry->cb.cb_addr)(file, reqprot, prot, flags), ret);
	return ret;
}

static int hook_socket_create(int family, int type, int protocol, int kern)
{
  int ret = 0;
  // TODO(jouyouyun): check cb addr validity
  EXEC_HOOK_CB(HOOK_MMAP_FILE,
               ((int (*)(void))entry->cb.cb_addr)(family, type, protocol, kern),
               ret);
  return ret;
}

static int hook_socket_listen(struct socket *sock, int backlog)
{
  int ret = 0;
  // TODO(jouyouyun): check cb addr validity
  EXEC_HOOK_CB(HOOK_MMAP_FILE,
               ((int (*)(void))entry->cb.cb_addr)(sock, backlog),
               ret);
  return ret;
}

static int __init manager_init(void)
{
	security_add_hooks(hooked_list, ARRAY_LIST(kooked_list), UOS_HOOK_MANAGER_LSM_NAME);
	pr_info("UOS Manager initialized: %s\n", UOS_HOOK_MANAGER_LSM_NAME);
	
	return 0;
}

security_initcall(manager_init);
