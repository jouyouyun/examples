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
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#define MAX_BUF_SIZE 2048

struct test_list_entry {
	char *owner;
	struct list_head link;
};

struct test_list_controller {
	struct mutex lock;
	struct list_head list;
};

static ssize_t sysfs_api_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_api_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count);
static ssize_t sysfs_list_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

static void query_filepath(const char *filename);
static void test_random(void);

static int init_test_list(void);
static void free_test_list(void);
static int add_test_list(const char *owner);
static void del_test_list(const char *owner);
static int list_test_list(void);
static int query_test_list(const char *owner);

static char _api_buf[MAX_BUF_SIZE] = {0};
static char *_api_list = "query_path random list/[add|del|list|query]/param";
static struct test_list_controller *_list_ctrl = NULL;

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
	else if (strstr(_api_buf, "list/add/") == _api_buf) {
		strcpy(cmd, _api_buf + 9);
		add_test_list(cmd);
	} else if (strstr(_api_buf, "list/del/") == _api_buf) {
		strcpy(cmd, _api_buf + 9);
		del_test_list(cmd);
	} else if (strstr(_api_buf, "list/list") == _api_buf) {
		list_test_list();
	} else if (strstr(_api_buf, "list/query/") == _api_buf) {
		strcpy(cmd, _api_buf + 11);
		query_test_list(cmd);
	}

	return count;
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

static int init_test_list(void)
{
	_list_ctrl = kzalloc(sizeof(struct test_list_controller), GFP_KERNEL);
	if (unlikely(!_list_ctrl))
		return -1;

	mutex_init(&_list_ctrl->lock);
	INIT_LIST_HEAD(&(_list_ctrl->list));

	return 0;
}

static void free_test_list(void)
{
  struct list_head *p = NULL, *next = NULL;
  struct test_list_entry *entry = NULL;

  mutex_lock(&_list_ctrl->lock);
  list_for_each_safe(p, next, &_list_ctrl->list) {
    entry = list_entry(p, struct test_list_entry, link);
    pr_info("[%s] will del: %s\n", __func__, entry->owner);
    list_del(&entry->link);
	kfree(entry->owner);
	kfree(entry);
  }
  mutex_unlock(&_list_ctrl->lock);
  mutex_destroy(&_list_ctrl->lock);
}

static int add_test_list(const char *owner)
{
	struct test_list_entry *entry = kzalloc(sizeof(struct test_list_entry), GFP_KERNEL);
	if (unlikely(!entry)) {
		pr_info("Failed to alloc mem in add list!");
		goto failure;
	}

	entry->owner = kzalloc(strlen(owner)+1, GFP_KERNEL);
	if (unlikely(!entry->owner)) {
		pr_info("Failed to alloc owner in add list!");
		goto failure;
	}
	
	strcpy(entry->owner, owner);
	mutex_lock(&_list_ctrl->lock);
	list_add_tail(&entry->link, &_list_ctrl->list);
	mutex_unlock(&_list_ctrl->lock);

	return 0;

 failure:
	if (entry)
		kfree(entry);
	
	return -1;
}

static void del_test_list(const char *owner)
{
	struct list_head *p = NULL, *next = NULL;
	struct test_list_entry *entry = NULL;

	mutex_lock(&_list_ctrl->lock);
	list_for_each_safe(p, next, &_list_ctrl->list) {
		entry = list_entry(p, struct test_list_entry, link);
		pr_info("[%s] will compare: %s -- %s\n", __func__, entry->owner, owner);
		if (strcmp(entry->owner, owner) != 0)
			continue;

		list_del(&entry->link);
		kfree(entry->owner);
		kfree(entry);
		break;
	}
	mutex_unlock(&_list_ctrl->lock);
}

static int list_test_list(void)
{
  struct list_head *p = NULL, *next = NULL;
  struct test_list_entry *entry = NULL;

  mutex_lock(&_list_ctrl->lock);
  list_for_each_safe(p, next, &_list_ctrl->list) {
    entry = list_entry(p, struct test_list_entry, link);
    pr_info("[%s] entry: %s\n", __func__, entry->owner);
  }
  mutex_unlock(&_list_ctrl->lock);

  return 0;
}

static int query_test_list(const char *owner)
{
	int found = 0;
	struct list_head *p = NULL, *next = NULL;
	struct test_list_entry *entry = NULL;

	mutex_lock(&_list_ctrl->lock);
	list_for_each_safe(p, next, &_list_ctrl->list) {
		entry = list_entry(p, struct test_list_entry, link);
		pr_info("[%s] will compare: %s -- %s\n", __func__, entry->owner, owner);
		if (strcmp(entry->owner, owner) != 0)
			continue;

		found = 1;
		pr_info("[%s] found: %s -- %s\n", __func__, entry->owner, owner);
		break;
	}
	mutex_unlock(&_list_ctrl->lock);
	return found;
}

int __init init_module()
{
	pr_info("Will init api_test\n");
	if (unlikely(init_test_list() != 0)) {
		pr_info("Failed to init test list, exit.");
		return -1;
	}
	
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

	pr_info("Init api_test success\n");
	return 0;

list_sysfs:
	sysfs_remove_file(kobj_api, &api_attr.attr);
api_sysfs:
	kobject_put(kobj_api);

	pr_info("Init api_test failed\n");
	return -1;
}

void __exit cleanup_module()
{
	free_test_list();
	sysfs_remove_file(kobj_api, &api_attr.attr);
	sysfs_remove_file(kobj_api, &list_attr.attr);
	kobject_put(kobj_api);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("Test kernel api");
