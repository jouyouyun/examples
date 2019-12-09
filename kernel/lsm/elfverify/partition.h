#pragma once

int is_whitelist_mountpoint(const char *mp);
void get_root(char* root,
	      unsigned int major, unsigned int minor);
void lsm_add_partition(const char* dir_name,
		       unsigned int major, unsigned int minor);
int get_major_minor_from_sysfs(const char *dev_name,
			       unsigned int* major, unsigned int* minor);
int is_special_mp(const char* mp);
int is_mnt_ns_valid(void);
int parse_mounts_info(char* buf);
