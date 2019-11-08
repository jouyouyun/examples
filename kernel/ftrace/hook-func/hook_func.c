#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/ftrace.h>
#include <linux/kallsyms.h>

struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

static int success = 0;

#define HOOK(_name, _function, _original)		\
	{						\
		.name = (_name),			\
			.function = (_function),	\
			.original = (_original),	\
			}

static int (*real_mmap_file)(struct file *file, unsigned long prot,
			     unsigned long flags);

static int hook_mmap_file(struct file *file, unsigned long prot,
			  unsigned long flags)
{
	int ret;

	pr_info("[mmap_file] hook start\n");
	ret = real_mmap_file(file, prot, flags);
	pr_info("[mmap_file] hook done, ret: %d\n", ret);

	return ret;
}

static struct ftrace_hook hooked_functions[] = {
	HOOK("security_mmap_file", hook_mmap_file, &real_mmap_file),
	HOOK(NULL, NULL, NULL),
};

static int resolve_hook_addr(struct ftrace_hook *hook)
{
	hook->address = kallsyms_lookup_name(hook->name);
	if (!hook->address) {
		pr_err("[resolve addr] failed for: %s\n", hook->name);
		return -ENOENT;
	}

	*((unsigned long*)hook->original) = hook->address;
	return 0;
}

static void notrace mmap_file_cb(unsigned long ip, unsigned long parent_ip,
                                 struct ftrace_ops *ops, struct pt_regs *regs)
{
	pr_info("[mmap_file] start, ip: 0x%lx, parent_ip: 0x%lx\n", ip, parent_ip);
	if (!regs) {
		pr_info("[%s] invalid regs\n", __func__);
		return ;
	}
	struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

	if (!within_module(parent_ip, THIS_MODULE)) {
		pr_info("[mmap_file_cb:] %s not module\n", hook->name);
#ifdef CONFIG_ARM64
		pr_info("[mmap_file_cb] ------------ ARM64 ---------\n");
		if (hook)
			regs->pc = (unsigned long)hook->function;
#else
		pr_info("[mmap_file_cb] ------------ NORMAL ---------\n");
		if (hook)
			regs->ip = (unsigned long)hook->function;
#endif
	}
}

static int install_hook(struct ftrace_hook *hook)
{
	int err;

	pr_info("[install_hook] start: %s\n", hook->name);
	err = resolve_hook_addr(hook);
	if (err) {
		return err;
	}
	pr_info("[install_hook] resolve addr: 0x%lx\n", hook->address);

	hook->ops.func = mmap_file_cb;
	// if FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED set, will set
	// success when unsupport regs
	hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS |
		FTRACE_OPS_FL_IPMODIFY |
		FTRACE_OPS_FL_RECURSION_SAFE;

	pr_info("[install_hook] set filter ip\n");
	err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
	pr_info("[install_hook] set filter ip, ret: %d\n", err);
	if (err) {
		// using 'pr_err' will block
		pr_warn("[install_hook] failed to install: %s\n", hook->name);
		return err;
	}

	pr_info("[install_hook] register\n");
	err = register_ftrace_function(&hook->ops);
	if (err) {
		pr_err("[install_hook] failed to register: %s, 0x%lx\n", hook->name, hook->address);
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
	}
	return err;
}

static void remove_hook(struct ftrace_hook *hook)
{
	int err;

	err = unregister_ftrace_function(&hook->ops);
	if (err) {
		pr_err("[install_hook] failed to unregister: %s, 0x%lx\n", hook->name, hook->address);
	}
	err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
	if (err) {
		pr_err("[install_hook] failed to set filter: %s, 0x%lx\n", hook->name, hook->address);
	}
}

int __init init_module()
{
	int i = 0;
	int err;
	pr_info("Init hook func\n");

	for (; hooked_functions[i].name; i++) {
		pr_info("[Init] start hook: %s\n", hooked_functions[i].name);
		err = install_hook(&hooked_functions[i]);
		if (err) {
			continue;
		}
		pr_info("[Init] success to add: %s\n", hooked_functions[i].name);
		success++;
	}
	return 0;
}

void __exit cleanup_module()
{
	int i = 0;
	pr_info("Exit hook func: %d\n", success);
	if (!success) {
		return;
	}
	for (; hooked_functions[i].name; i++) {
		remove_hook(&hooked_functions[i]);
	}
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("Test ftrace hook");;
