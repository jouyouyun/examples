#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/version.h>
#include <uapi/asm-generic/mman-common.h>

static unsigned long get_arg(struct pt_regs* regs, int n)
{
	switch (n) {
#if    defined(CONFIG_X86_64)

	case 1: return regs->di;
	case 2: return regs->si;
	case 3: return regs->dx;
	case 4: return regs->cx;
	case 5: return regs->r8;
	case 6: return regs->r9;

#elif defined(CONFIG_ARM64)

	case 1:  // x0
	case 2:  // x1
	case 3:  // x2
	case 4:  // x3
		return *(unsigned long*)((char *)regs + (3+n)*8);

#endif // CONFIG_X86_64
	default:
		return 0;
	}
	return 0;
}

static void notrace my_ftrace_func(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs)
{
	if (!regs) {
		pr_info("[%s] invalid regs\n", __func__);
		return;
	}
	struct filename* fn = (struct filename*)get_arg(regs, 2);
	pr_info("%pf called from %pf: %s\n", (void *)ip, (void *)parent_ip, fn->name);
}

static struct ftrace_ops fops = {
	.func = my_ftrace_func,
	.flags = FTRACE_OPS_FL_SAVE_REGS,
};

int __init init_module()
{
	char* fname = "do_filp_open";
	int ret = ftrace_set_filter(&fops, fname, strlen(fname), 0);
	if (ret) {
		pr_err("ftrace-set-filter error: %d\n", ret);
		return ret;
	}

	ret = register_ftrace_function(&fops);
	if (ret) {
		pr_err("reg-ftrace-func error: %d\n", ret);
		return ret;
	}

	pr_info("reg-ftrace-peek done\n");
	return 0;
}

void __exit cleanup_module()
{
	unregister_ftrace_function(&fops);
	pr_info("unreg-ftrace-peek done\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raphael");
MODULE_DESCRIPTION("ftrace function test kernel module");
