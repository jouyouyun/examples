/**
 * Copyright (C) 2020 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * lsm_demo.c -- lsm demo
 *
 * Written on 星期四, 24 十二月 2020.
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/lsm_hooks.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
#include <uapi/linux/mman.h>
#else
#include <uapi/asm-generic/mman-common.h>
#endif

#define DEMO_LSM_NAME "lsm_demo"

static int hook_file_ioctl(struct file *file, unsigned int cmd,
                           unsigned long arg) {
	pr_info("file ioctl hooked");
	return 0;
}

extern struct security_hook_heads security_hook_heads;
extern void security_add_hooks(struct security_hook_list *hooks, int count,
                               char *lsm);

static int demo_lsm_init(void)
{
	unsigned long head_addr = 0;
	unsigned long add_addr = 0;
	struct security_hook_heads *head = NULL;

	head_addr = kallsyms_lookup_name("security_hook_heads");
	if (head_addr == 0) {
		pr_info("Failed to lookup security_hook_heads");
		return -2;
	}
	head = (struct security_hook_heads*)head_addr;

	struct security_hook_list hooked_list[] = {
		{
			.head = &(head->file_ioctl),
			.hook = {.file_ioctl = hook_file_ioctl},
		},
	};
	
	add_addr = kallsyms_lookup_name("security_add_hooks");
	if (add_addr == 0) {
		pr_info("Failed to lookup security_add_hooks");
		return -1;
	}
	((void (*)(struct security_hook_list*,
		   int, char*))add_addr)(hooked_list, ARRAY_SIZE(hooked_list), DEMO_LSM_NAME);
	pr_info("Load lsm_demo success");
	return 0;
}

int __init demo_init(void)
{
/*
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	DEFINE_LSM(lsm_demo) = {
		.name = DEMO_LSM_NAME,
		.init = demo_lsm_init,
	};
#else
	security_initcall(demo_lsm_init);
#endif
*/

	pr_info("Will load lsm_demo");
	return demo_lsm_init();
}

void __exit cleanup_module()
{
	pr_info("exit lsm demo");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("LSM demo");
