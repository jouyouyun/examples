#include <linux/buffer_head.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>

#define ALLOC_UNIT (1 << 12)

static char *dest_file = "/var/lib/deepin/developer-mode/enabled";
module_param(dest_file, charp, 0644);
MODULE_PARM_DESC(dest_file, "the target file to read");

/*
  kernel_read_file_from_path cant be used here because:
  1. procfs doesnt have i_size, which is used by kernel_read_file, which is in turn called by k_r_f_f_p
  2. we want to avoid security_XXX calls
*/
char *read_file_content(const char *filename, int *real_size)
{
	struct file *filp = NULL;
	char *buf = NULL;
	mm_segment_t old_fs;
	loff_t off = 0;
	int size = ALLOC_UNIT;

	if (unlikely(filename == NULL) || unlikely(real_size == NULL))
		return NULL;

	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
   	        pr_warn("[%s] failed to open: %s\n", __func__, filename);
		return NULL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	while(1) {
		buf = kzalloc(size, GFP_KERNEL);
		if (unlikely(buf == NULL))
			break;

		off = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
		*real_size = kernel_read(filp, buf, size, &off);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
		*real_size = vfs_read(filp, (char __user*)buf, size, &off);
#else
		*real_size = __vfs_read(filp, (char __user*)buf, size, &off);
#endif
		if (*real_size > 0 && *real_size < size) {
			buf[*real_size] = 0;
			break;
		}

		kfree(buf);
		buf = NULL;
		if (*real_size != 0)
			size += ALLOC_UNIT;
		else
			break;
	}

	set_fs(old_fs);
	filp_close(filp, 0);
	return buf;
}

int __init init_module(void)
{
	char *content = NULL;
	int size = 0;
	
	pr_info("Will init module, file: %s\n", dest_file);
	content = read_file_content(dest_file, &size);
	if (!content) {
		return -1;
	}
	
	pr_info("Init module done, content: '%s', size: %d\n", content, size);
	kfree(content);
	return 0;
}

void __exit cleanup_module(void)
{
	pr_info("Exit module\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("Test file operations");
