#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "common.h"

#define FILEPATH(_ty, _path) {					\
		switch (_ty) {					\
		case TY_PASSWD: _path = PASSWD_FILE; break;	\
		case TY_GROUP: _path = GROUP_FILE; break;	\
		case TY_SHADOW: _path = SHADOW_FILE; break;	\
		default: break;					\
		}						\
	}

#define make_passwd_info(_strv, _info)					\
	{								\
		passwd_info *pass = (passwd_info *)_info;		\
		memset(pass->user, 0, MAX_NAME_LEN);			\
		memset(pass->home, 0, MAX_PATH_LEN);			\
		memset(pass->shell, 0, MAX_SHELL_LEN);			\
		memcpy(pass->user, _strv[0], MAX_NAME_LEN - 1);		\
		memcpy(pass->home, _strv[3], MAX_PATH_LEN - 1);		\
		memcpy(pass->shell, _strv[4], MAX_SHELL_LEN - 1);	\
		pass->uid = strtoul(_strv[1], NULL, 10);		\
		pass->gid = strtoul(_strv[2], NULL, 10);		\
	}

#define make_group_info(_strv, _info)					\
	{								\
		int i = 0;						\
		int size = 0;						\
		char **list = NULL;					\
		group_info *group = (group_info *)_info;		\
		memset(group->group, 0, MAX_NAME_LEN);			\
		memcpy(group->group, _strv[0], MAX_NAME_LEN - 1);	\
		group->gid = strtoul(_strv[1], NULL, 10);		\
		list = split(_strv[2], ",", &size);			\
		if (size != 0) {					\
			group->group_list = (char **)malloc(sizeof(char *) * (size + 1)); \
			for (; i < size; i++) {				\
				group->group_list[i] = calloc(MAX_NAME_LEN, sizeof(char)); \
				memcpy(group->group_list[i], list[i], MAX_NAME_LEN - 1); \
			}						\
			free(list);					\
			group->group_list[i] = NULL;			\
		}							\
	}

#define make_shadow_info(_strv, _info)					\
	{								\
		shadow_info *shadow = (shadow_info *)_info;		\
		memset(shadow->user, 0, MAX_NAME_LEN);			\
		memset(shadow->passwd, 0, MAX_PASSWD_LEN);		\
		memcpy(shadow->user, _strv[0], MAX_NAME_LEN - 1);	\
		memcpy(shadow->passwd, _strv[1], MAX_PASSWD_LEN - 1);	\
	}

static char **split(const char *str, const char *delim, int *size)
{
	int i = 0;
	char **strv = NULL;
	char *tmp = NULL;

	*size = 0;
	tmp = strtok((char*)str, delim);
	if (!tmp)
		return NULL;

	strv = (char **)malloc(sizeof(char*));
	strv[i] = tmp;

	while ((tmp = strtok(NULL, delim)) != NULL) {
		strv = (char **)realloc(strv, sizeof(char *) * ((++i)+1));
		strv[i] = tmp;
	}

	// trim '\n'
	if (strv[i][strlen(strv[i])-1] == '\n')
		strv[i][strlen(strv[i])-1] = '\0';

	*size = i+1;
	return strv;
}

static int do_query_by_name(const char *name, const enum FILE_TYPE ty,
                            const char **strv, void *info) {
	if (strcmp(strv[0], name) != 0) {
		return -1;
	}

	if (ty == TY_PASSWD) {
		make_passwd_info(strv, info);
        } else if (ty == TY_GROUP) {
		make_group_info(strv, info);
        } else if (ty == TY_SHADOW) {
		make_shadow_info(strv, info);
        }

        return 0;
}

static int do_query_by_id(const unsigned int id, const enum FILE_TYPE ty,
                          const char **strv, void *info) {
	unsigned long tmp_id = 0;

	tmp_id = strtoul(strv[1], NULL, 10);
	if (tmp_id != id) {
		return -1;
	}

	if (ty == TY_PASSWD) {
		make_passwd_info(strv, info);
	} else if (ty == TY_GROUP) {
		make_group_info(strv, info);
	}
	
	return 0;
}

static int do_query_by_idx(const enum FILE_TYPE ty, const char **strv, void *info)
{
	if (ty == TY_PASSWD) {
		make_passwd_info(strv, info);
	} else if (ty == TY_GROUP) {
		make_group_info(strv, info);
	} else if (ty == TY_SHADOW) {
		make_shadow_info(strv, info);
	}

        return 0;
}

static int do_query(const char *name, unsigned int id, unsigned int idx,
		    const enum FILE_TYPE ty, void *info)
{
	int ret = 0, found = 0;
	int i = 1;
	FILE *fr = NULL;
	char *file = NULL;
	char line[MAX_PASSWD_LEN];

	FILEPATH(ty, file);
	assert(file);

	fr = fopen(file, "r");
	if (!fr) {
		return errno;
	}

	do {
		ret = -1;
		memset(line, 0, MAX_PASSWD_LEN);
		if (fgets(line, MAX_PASSWD_LEN, fr) == NULL) {
			ret = errno;
			break;
		}

		int size = 0;
		char *tmp = line;
		char **strv = split(tmp, ":", &size);
		if (!strv) {
			// empty line or comment
			WEN_INFO("invalid file format: %s", line);
			continue;
		}

		if (name != NULL) {
			ret = do_query_by_name(name, ty, (const char**)strv, info);
			found = 1;
                } else if (id != 0) {
			ret = do_query_by_id(id, ty, (const char **)strv, info);
			found = 1;
                } else if (idx == i) {
			ret = do_query_by_idx(ty, (const char **)strv, info);
			found = 1;
		}

		i++;
		free(strv);
                if (found == 1 && ret == 0)
			break;
        } while (1);

        fclose(fr);
	return ret;
}

int query(const char *name, const enum FILE_TYPE ty, void *info)
{
	WEN_INFO("Will query by name: %s, ty: %d", name, ty);
	return do_query(name, 0, 0, ty, info);
}

int query_by_id(unsigned int id, const enum FILE_TYPE ty, void *info)
{
	assert(ty == TY_PASSWD || ty == TY_GROUP);
        WEN_INFO("Will query by id: %u, ty: %d", id, ty);
        return do_query(NULL, id, 0, ty, info);
}

int query_by_idx(const unsigned int idx, const enum FILE_TYPE ty, void *info)
{
	WEN_INFO("Will query by idx: %u, ty: %d", idx, ty);
	return do_query(NULL, 0, idx, ty, info);
}

void free_group_list(char **list)
{
	int i = 0;
        for (; list[i]; i++) {
		printf("[%s] will free %s\n", __func__, list[i]);
		free(list[i]);
        }
        free(list);
}
