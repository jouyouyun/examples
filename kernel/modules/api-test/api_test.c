#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/string.h>
#include <linux/random.h>

#define MAX_BUF_SIZE (size_t)1024

static ssize_t sysfs_api_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_api_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count);
static ssize_t sysfs_list_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

static void query_filepath(const char *filename);
static void test_random(void);

static char _api_buf[MAX_BUF_SIZE] = {0};
static char *_api_list = "query_path random";

static struct kobject *kobj_api = NULL;
static struct kobj_attribute api_attr = __ATTR(_api_buf, 0660, sysfs_api_show, sysfs_api_store);
static struct kobj_attribute list_attr = __ATTR(_api_list, 0440, sysfs_list_show, NULL);

static ssize_t sysfs_api_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	pr_info("[%s] read\n", __func__);
	return snprintf(buf, MAX_BUF_SIZE, "%s\n", _api_buf);
}

static ssize_t sysfs_api_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count)
{
	char cmd[MAX_BUF_SIZE] = {0};

	pr_info("[%s] write\n", __func__);
	memset(_api_buf, 0, MAX_BUF_SIZE);
	sscanf(buf, "%s", _api_buf);

	pr_info("[%s] Content: '%s'\n", __func__, _api_buf);
	if (strstr(_api_buf, "query_path/") == _api_buf) {
		strcpy(cmd, _api_buf + 10);
		query_filepath(cmd);
	} else if (strstr(_api_buf, "random") == _api_buf)
		test_random();
	return (ssize_t)count;
}

static ssize_t sysfs_list_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	pr_info("[%s] read\n", __func__);
	return snprintf(buf, MAX_BUF_SIZE, "%s\n", _api_list);
}

static void query_filepath(const char *filename)
{
	int error = 0;
	struct path path;

	error = kern_path(filename, LOOKUP_FOLLOW, &path);
	if (error) {
		pr_info("[%s] failed to query: %d\n", __func__, error);
		return ;
	}

	pr_info("[%s] Print dentry\n", __func__);
	if (likely(path.dentry)) {
		pr_info("[%s] Name: %s, symlink: %s\n", __func__, path.dentry->d_name.name,
			d_is_symlink(path.dentry)?"yes":"no");
			/* (path.dentry->d_flags & DCACHE_ENTRY_TYPE == DCACHE_SYMLINK_TYPE)?"yes":"no"); */
		if (likely(path.dentry->d_sb))
			pr_info("[%s] Device: %u\n", __func__, path.dentry->d_sb->s_dev);
	}
}

static void test_random(void)
{
	int i = 0;
	unsigned int num32 = 0;
	unsigned long num64 = 0;

	for (; i < 10; i++) {
		num32 = get_random_u32();
		pr_info("[%s] Random: %d - %u\n", __func__, i, num32);
	}

	i = 0;
	for (; i < 10; i++) {
		num64 = get_random_u64();
		pr_info("[%s] Random: %d - %lu\n", __func__, i, num64);
	}
}

int __init init_module()
{
	kobj_api = kobject_create_and_add("api_test", kernel_kobj);
	if (unlikely(kobj_api == NULL)) {
		pr_info("Failed to init, exit.");
		return -1;
	}

	if (sysfs_create_file(kobj_api,&api_attr.attr)) {
		pr_info("[%s] failed to create api attribute\n", __func__);
		goto api_sysfs;
	}

	if (sysfs_create_file(kobj_api,&list_attr.attr)) {
		pr_info("[%s] failed to create list attribute\n", __func__);
		goto list_sysfs;
	}

	return 0;

list_sysfs:
	sysfs_remove_file(kobj_api, &api_attr.attr);
api_sysfs:
	kobject_put(kobj_api);

	return -1;
}

void __exit cleanup_module()
{
	sysfs_remove_file(kobj_api, &api_attr.attr);
	sysfs_remove_file(kobj_api, &list_attr.attr);
	kobject_put(kobj_api);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("Test kernel api");
