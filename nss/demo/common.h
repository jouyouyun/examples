#pragma  once

#include <systemd/sd-journal.h>

#define PASSWD_FILE "/etc/wen/passwd" // user:uid:gid:home:shell
#define GROUP_FILE "/etc/wen/group" // group:gid:group1,group2
#define SHADOW_FILE "/etc/wen/shadow" // user:hash-of-password

#define MAX_NAME_LEN 32
#define MAX_PATH_LEN 40
#define MAX_SHELL_LEN 10
#define MAX_PASSWD_LEN 65

enum FILE_TYPE {
	TY_PASSWD,
	TY_GROUP,
	TY_SHADOW
};

typedef struct _passwd_info_ {
	char user[MAX_NAME_LEN];
	unsigned int uid;
	unsigned int gid;
	char home[MAX_PATH_LEN];
	char shell[MAX_SHELL_LEN];
} passwd_info;

typedef struct _group_info_ {
	char group[MAX_NAME_LEN];
	unsigned int gid;
	char **group_list; // termiante by NULL
	unsigned int mem_len;
} group_info;

typedef struct _shadow_info_ {
	char user[MAX_NAME_LEN];
	char passwd[MAX_PASSWD_LEN];
} shadow_info;

int query(const char *name, const enum FILE_TYPE ty, void *info);
int query_by_id(const unsigned int id, const enum FILE_TYPE ty, void *info);
int query_by_idx(const unsigned int idx, const enum FILE_TYPE ty, void *info);

void free_group_list(char **list);

passwd_info *make_passwd_from_data(char *data);
group_info *make_group_from_data(char *data);

#define WEN_INFO(fmt, ...) sd_journal_print(LOG_INFO, fmt, ##__VA_ARGS__)
