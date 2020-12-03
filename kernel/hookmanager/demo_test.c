#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/lsm_uos_hook_manager.h>

#define MODULE_NAME "demo_test"

typedef struct td_hook_entry {
	int hook_id;
	struct uos_hook_cb_entry cb;
}td_hook_entry_t;

int td_path_rename(const struct path *old_dir, struct dentry *old_dentry,const struct path *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	printk("curent: %s %s: test\n", current->comm, __FUNCTION__);

	return 0;
}
int td_path_mkdir (const struct path *dir, struct dentry *dentry, umode_t mode)
{
	printk("curent: %s %s: test\n", current->comm, __FUNCTION__);

	return 0;
}

static td_hook_entry_t entries[] = {
#if 1
	{
		.hook_id = UOS_PATH_RENAME,
		.cb =
			{
				.owner = MODULE_NAME,
				.cb_addr = (unsigned long)td_path_rename,
				.ret_type = UOS_HOOK_RET_TY_INT,
				.arg_len = 4,
			},
	},
#endif
#if 1
	{
		.hook_id = UOS_PATH_MKDIR,
		.cb =
			{
				.owner = MODULE_NAME,
				.cb_addr = (unsigned long)td_path_mkdir,
				.ret_type = UOS_HOOK_RET_TY_INT,
				.arg_len = 3,
			},
	},
#endif
    {
        .hook_id = UOS_HOOK_NONE,
    },
};


int td_uos_manager_register_hook(void)
{
	int i = 0, j = 0;
	int error = 0;

	for (; entries[i].hook_id != UOS_HOOK_NONE; i++) {
		error = uos_hook_register(entries[i].hook_id, &entries[i].cb);
		if (error) {
			printk("Failed to register hook %d, hook_id = %d\n", i, entries[i].hook_id);
			break;
		}
	}

	if (entries[i].hook_id == UOS_HOOK_NONE)
		return 0;

	for (; j < i; j++) {
		error = uos_hook_cancel(entries[j].hook_id, entries[j].cb.owner);
		if (error)
			printk("Failed to cancel hook %d\n", j);
	}

	return -1;
}

void td_uos_manager_cancel_hook(void)
{
	int i = 0;
	int error = 0;

	for (; entries[i].hook_id != UOS_HOOK_NONE; i++) {
		error = uos_hook_cancel(entries[i].hook_id, entries[i].cb.owner);
		if (error)
			printk("Failed to cancel hook %d, hook_id = %d\n", i, entries[i].hook_id);
        }
}

int __init init_module(void)
{
	int error = 0;

#ifdef CONFIG_SECURITY_PATH
	printk("CONFIG_SECURITY_PATH is defined\n");
#endif


	pr_info("start to init uos hook demo\n");
	error = td_uos_manager_register_hook();
	if (error) {
		pr_info("failed to registe hook\n");
		return -1;
	}
	
        pr_info("finish to init uos hook demo\n");
	return 0;
}

void __exit cleanup_module(void)
{
	td_uos_manager_cancel_hook();
	pr_info("exit the uos hook demo\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("The demo for uos lsm hook manager");


