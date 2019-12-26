#include <linux/path.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/nsproxy.h>
#include <linux/namei.h>
#include <linux/buffer_head.h>

#include "partition.h"
#include "utils.h"

typedef struct __verify_partition__ {
	unsigned int major, minor;
	struct list_head list;
	char root[0];
} verify_partition;

static DECLARE_RWSEM(rw_parts);
static LIST_HEAD(partitions);

int is_whitelist_mountpoint(const char *mp)
{
	static char *whitelist[] = {"/", "/boot", "/usr", NULL};
	int i = 0;

	for (; whitelist[i]; i++) {
		if (strcmp(whitelist[i], mp) == 0)
			return 1;
	}

	return 0;
}

void get_root(char* root,
	      unsigned int major, unsigned int minor)
{
	verify_partition *part = NULL;

	// TODO(jouyouyun): device has multi mount points
	*root = 0;
	down_read(&rw_parts);
	list_for_each_entry(part, &partitions, list) {
		if (part->major == major && part->minor == minor) {
			strcpy(root, part->root);
			break;
		}
	}
	up_read(&rw_parts);
}

void lsm_add_partition(const char* dir_name,
		       unsigned int major, unsigned int minor)
{
	verify_partition *part = NULL;

	part = kzalloc(sizeof(verify_partition) + strlen(dir_name) + 1, GFP_KERNEL);
	if (unlikely(part == 0)) {
		pr_err("kzalloc failed and thus cant add %s [%d, %d] to partitions\n",
		       dir_name, major, minor);
		return;
	}

	part->major = major;
	part->minor = minor;
	strcpy(part->root, dir_name);
	down_write(&rw_parts);
	list_add_tail(&part->list, &partitions);
	up_write(&rw_parts);
}

int get_major_minor_from_sysfs(const char *dev_name,
			       unsigned int* major, unsigned int* minor)
{
	char *content = NULL;
	char filename[NAME_MAX + 21] = {0};
	int size = 0;
	unsigned int ma = 0, mi = 0;

	if (strstr(dev_name, "/dev/") != dev_name)
		return 1;

	memset(filename, 0, NAME_MAX + 21);
	sprintf(filename, "/sys/class/block/%s/dev", dev_name + 5);
	content = read_file_content(filename, &size);
	if (!content)
		return 1;

	if (content[size-1] == '\n')
		content[size-1] = '\0';
	if (sscanf(content, "%u:%u", &ma, &mi) != 2) {
		kfree(content);
		content = NULL;
		return 1;
	}

	kfree(content);
	content = NULL;

	if (ma == 0 || mi == 0)
		return 1;

	*major = ma;
	*minor = mi;
	return 0;
}

int parse_mounts_info(char* buf)
{
	verify_partition* part = NULL;
	char *mp = NULL;
	char *line = NULL;
	unsigned int major = 0, minor = 0;
	int parts_count = 0;

	mp = kzalloc(NAME_MAX, GFP_KERNEL);
	if (unlikely(mp == NULL))
		return -1;

	line = buf;
	while (sscanf(line, "%*d %*d %u:%u %*s %250s %*s %*s %*s %*s %*s %*s\n", &major, &minor, mp) == 3) {
		line = strchr(line, '\n') + 1;

		if (is_special_mp(mp)) {
			memset(mp, 0, NAME_MAX);
			continue;
		}

		part = kzalloc(sizeof(verify_partition) + strlen(mp) + 1, GFP_KERNEL);
		if (unlikely(part == 0)) {
			pr_err("verify-partition kzalloc failed for %s\n", mp);
			memset(mp, 0, NAME_MAX);
			continue;
		}
		part->major = major;
		part->minor = minor;
		strcpy(part->root, mp);
		memset(mp, 0, NAME_MAX);
		list_add_tail(&part->list, &partitions);
	}
	kfree(mp);
	mp = NULL;

	// __init section doesnt need lock
	part = NULL;
	list_for_each_entry(part, &partitions, list) {
		parts_count++;
		pr_info("mp: %s, major: %d, minor: %d\n", part->root, part->major, part->minor);
	}
	return parts_count;
}

static int mounted_at(const char* mp, const char* root)
{
	return strcmp(mp, root) == 0 ||
		(strlen(mp) > strlen(root) && strstr(mp, root) == mp && mp[strlen(root)] == '/');
}

int is_special_mp(const char *mp)
{
	static char *mp_list[] = {"/sys", "/proc", "/run", "/dev", NULL};
	int i = 0;

	if (!mp || *mp != '/')
		return 1;

	for (; mp_list[i]; i++)
		if (mounted_at(mp, mp_list[i]))
			return 1;

	return 0;
}

int is_mnt_ns_valid(void)
{
	static struct mnt_namespace* init_mnt_ns = NULL;

	if (init_mnt_ns == 0) {
		if (current && current->nsproxy)
			init_mnt_ns = current->nsproxy->mnt_ns;
		return init_mnt_ns != NULL;
	}

	if (current && current->nsproxy && current->nsproxy->mnt_ns != init_mnt_ns)
		return 0;

	return 1;
}
