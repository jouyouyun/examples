/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * template.c -- template
 *
 * Written on 星期一,  4 一月 2021.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define MOD_NAME "template"

int __init mod_init(void)
{
	printk("[%s] init", MOD_NAME);
	return 0;
}

void __exit mod_exit(void)
{
	printk("[%s] exit", MOD_NAME);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("template");
