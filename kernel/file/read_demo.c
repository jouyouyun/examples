/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * rw_demo.c -- kernel read/write file demo
 *
 * Written on 星期五,  1 一月 2021.
 */

#include <linux/buffer_head.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>

#define MODULE_NAME "read_demo"
#define DEMO_FILE "/proc/cmdline"
#define BUF_SIZE (1<<12)

static char *read_file_content(const char *filename, int *real_size)
{
	struct file *filp = NULL;
	char *buf = NULL;
	loff_t off = 0;
	int size = BUF_SIZE;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
        mm_segment_t old_fs;
#endif

        filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		printk("[%s] failed to open: %s\n", MODULE_NAME, filename);
		return NULL;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
        old_fs = get_fs();
        set_fs(get_fs());
#endif

        while (1) {
		buf = kzalloc(size, GFP_KERNEL);
                if (unlikely(buf == NULL)) {
			printk("[%s] alloc memory failed\n", MODULE_NAME);
			break;
                }

		off = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
                *real_size = kernel_read(filp, buf, size, &off);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
                *real_size = vfs_read(filp, (char __user *)buf, size, &off);
#else
                *real_size = __vfs_read(filp, (char __user *)buf, size, &off);
#endif
                if (*real_size > 0 && *real_size < size) {
			buf[*real_size] = 0;
			break;
                }

                kfree(buf);
                buf = NULL;
                if (*real_size != 0)
			size += BUF_SIZE;
                else
			break;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
        set_fs(old_fs);
#endif
        filp_close(filp, 0);
        return buf;
}

int __init mod_init(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	char __kernel *content = NULL;
#else
	char *content = NULL;
#endif
	int size = 0;

	printk("[%s] will read '%s'", MODULE_NAME, DEMO_FILE);
	content = read_file_content(DEMO_FILE, &size);
	if (unlikely(content == NULL)) {
		printk("[%s] failed to read file", MODULE_NAME);
		return -1;
	}

	printk("%s load success, read '%d' byte data: %s\n", MODULE_NAME, size,
	       content);
	kfree(content);

	return 0;
}

void __exit mod_exit(void)
{
	printk("[%s] exit", MODULE_NAME);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("ELF guard implemented by ftrace");
MODULE_LICENSE("GPL");
