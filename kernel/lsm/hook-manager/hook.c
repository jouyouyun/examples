/**
 * Copyright (C) 2020 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * hook.c -- The hook common methods of hook manager
 *
 * Written on 星期五, 26 六月 2020.
 */

#include "hook.h"

#define APPEND_HOOK_CB(hook, cb)										\
	{																	\
		hook_entry_list *entries = &hooked_entries[hook];				\
		mutex_lock(&entries->lock);										\
		list_add_tail(&cb.list, &entries->entries);						\
		mutex_unlock(&entries->lock);									\
	}

#define QUERY_HOOK_CB(hook, owner, found) \
	{													  \
		hook_entry *entry = NULL;						  \
		struct list_head *p = NULL, *next = NULL;		  \
		hook_entry_list *entries = &hooked_entries[hook]; \
		mutex_lock(&entries->lock);						  \
		list_for_each_safe(p, next, &entries->entries) {  \
			entry = list_entry(p, hook_entry, list);	  \
			if (strcmp(entry->cb.owner, owner) == 0) {	  \
				found = 1;								  \
				break;									  \
			}											  \
		}												  \
		mutex_unlock(&entries->lock);					  \
	}

int hook_register(int hook_name, struct hook_cb_entry *entry)
{
	int found = 0;

	QUERY_HOOK_CB(hook_name, entry->owner, found);
	if (found)
		return 0;

	return 0;
}

int hook_cancel(int hook_name, char *owner)
{
	int found = 0;

	QUERY_HOOK_CB(hook_name, entry->owner, found);
	if (!found)
		return 0;

	return 0;
}
