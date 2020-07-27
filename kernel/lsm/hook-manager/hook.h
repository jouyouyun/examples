/**
 * Copyright (C) 2020 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * hook.h -- The hook common defines of hook manager
 *
 * Written on 星期五, 26 六月 2020. */

#ifndef __HOOK_H__
#define __HOOK_H__

#include <linux/list.h>

enum HOOK_RETURN_TYPE {
	HOOK_RET_TY_NONE,
	HOOK_RET_TY_INT,
};

struct hook_cb_entry {
	char *owner; // the module name of the hook
	unsigned long cb_addr; // the callback address of the hook
	unsigned int ret_ty; // the return type of the hook
	unsigned int arg_len; // the argument length of the hook
};

typedef struct _hook_entry {
  struct hook_cb_entry cb;
  struct list_head list;
} hook_entry;

typedef struct _hook_entry_list {
  struct mutex lock;
  struct list_head entries;
} hook_entry_list;

hook_entry_list hooked_entries[3];

int hook_register(int hook_name, struct hook_cb_entry *entry);
int hook_cancel(int hook_name, char *owner);

enum HOOK_CB_LIST {
	HOOK_MMAP_FILE,
	HOOK_SOCKET_CREATE,
	HOOK_SOCKET_LISTEN,
};

#endif
