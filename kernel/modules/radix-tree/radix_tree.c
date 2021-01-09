/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * radix_tree.c -- kernel radix-tree example
 *
 * Written on 星期一,  4 一月 2021.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/radix-tree.h>

#define FLAG_R 1 << 0
#define FLAG_W 1 << 1
#define FLAG_D 1 << 2

struct file_node {
  unsigned long id;
  unsigned int flag;
  char *file;
};

#define MOD_NAME "radix_tree"
#define DEFAULT_FLAG (FLAG_R | FLAG_W)

static char *_files[] = {
	"/opt/dlp/data/wps/file1.doc",
	"/opt/dlp/data/wpa/config.ini",
	NULL,
};

static unsigned long id_list[2] = {0};

RADIX_TREE(_root, 0);

static struct file_node *make_file_node(const char *file, const int flag)
{
	static size_t node_len = sizeof(struct file_node);
	int len = strlen(file);
	struct file_node *node = kzalloc(node_len, GFP_KERNEL);

        if (unlikely(node == NULL))
		return NULL;

        node->file = kzalloc(len+1, GFP_KERNEL);
        if (unlikely(node->file == NULL))
		goto free;

        node->id = (unsigned long)node;
	node->flag = flag;
	memcpy(node->file, file, len);
	
	return node;
free:
	kfree(node);
	return NULL;
}

static void free_file_node(struct file_node *node)
{
	kfree(node->file);
	kfree(node);
}

static void lookup_file_node(unsigned long id)
{
	struct file_node *node = NULL;

	node = (struct file_node *)radix_tree_lookup(&_root, id);
	if (unlikely(node == NULL)) {
		printk("[%s] failed to lookup node: 0x%lx", MOD_NAME, id);
		return;
	}
	
	printk("[%s] node value: id(0x%lx), flag(%u), file(%s)", MOD_NAME, node->id,
	       node->flag, node->file);
}

int __init mod_init(void) {
	int i = 0;

	printk("[%s] init", MOD_NAME);
	while (_files[i] != NULL) {
		struct file_node *node = make_file_node(_files[i], DEFAULT_FLAG);
		if (unlikely(node == NULL)) {
			printk("[%s] failed to make file_node for '%s'", MOD_NAME, _files[i]);
			i++;
			continue;
		}

		printk("[%s] insert node: 0x%lx", MOD_NAME, node->id);
		radix_tree_insert(&_root, node->id, (void *)node);
		id_list[i] = node->id;
		i++;
	}

	lookup_file_node(id_list[1]);
	lookup_file_node(12334);
	return 0;
}

void __exit mod_exit(void)
{
	void **node_ptr = NULL;
        struct radix_tree_iter iter;

        printk("[%s] exit", MOD_NAME);
        radix_tree_for_each_slot(node_ptr, &_root, &iter, 0) {
		struct file_node *node = (struct file_node*)(*node_ptr);
                printk("[%s] iter node value: id(0x%lx), flag(%u), file(%s)", MOD_NAME, node->id,
                       node->flag, node->file);
                radix_tree_delete(&_root, node->id);
                free_file_node(node);
                node = NULL;
        }

	printk("[%s] empty: %d", MOD_NAME, radix_tree_empty(&_root));
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("a kernel radix-tree example");
