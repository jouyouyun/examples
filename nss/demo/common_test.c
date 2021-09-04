#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "common.h"
#include "message.h"

#undef PASSWD_FILE
#define PASSWD_FILE "./testdata/passwd"
#undef GROUP_FILE
#define GROUP_FILE "./testdata/group"
#undef SHADOW_FILE
#define SHADOW_FILE "./testdata/shadow"

static void test_passwd_query()
{
	int i = 0;
	int ret = 0;
	passwd_info info;
	char *names[] = {"wtest1", "wtest2", NULL};
	unsigned int ids[] = {5000, 5001, 0};
	unsigned int idx_list[] = {1, 2, 0};

	for (; names[i] != NULL; i++) {
		memset(&info, 0, sizeof(passwd_info));
		ret = query(names[i], TY_PASSWD, &info);
		assert(ret == 0);
		assert(strcmp(info.user, names[i]) == 0);
		printf("PASSWD: %s(%u:%u) -> %s -> %s\n", info.user, info.uid, info.gid,
		       info.home, info.shell);
        }

	for (i = 0; ids[i] != 0; i++) {
		memset(&info, 0, sizeof(passwd_info));
		ret = query_by_id(ids[i], TY_PASSWD, &info);
                assert(ret == 0);
                assert(info.uid == ids[i]);
                printf("PASSWD(id): %s(%u:%u) -> %s -> %s\n", info.user, info.uid, info.gid,
                       info.home, info.shell);
        }

	for (i = 0; idx_list[i] != 0; i++) {
		memset(&info, 0, sizeof(passwd_info));
		ret = query_by_idx(idx_list[i], TY_PASSWD, &info);
		assert(ret == 0);
                printf("PASSWD(idx): %s(%u:%u) -> %s -> %s\n", info.user,
                       info.uid, info.gid, info.home, info.shell);
        }
}

static void test_group_query() {
	int i = 0;
	int ret = 0;
	group_info info;
	char *names[] = {"wtest1", "wtest2", NULL};
	unsigned int ids[] = {5000, 5001, 0};
        unsigned int idx_list[] = {1, 2, 0};

        for (; names[i] != NULL; i++) {
		memset(&info, 0, sizeof(group_info));
		ret = query(names[i], TY_GROUP, &info);
		assert(ret == 0);
		assert(strcmp(info.group, names[i]) == 0);
		printf("GROUP: %s(%u) --> ", info.group, info.gid);
		int j = 0;
		for(; info.group_list[j] != NULL; j++) {
			printf("%s, ", info.group_list[j]);
		}
		if (j != 0)
			printf("\n");
	}
	free_group_list(info.group_list);

	for (i = 0; ids[i] != 0; i++) {
		memset(&info, 0, sizeof(group_info));
		ret = query_by_id(ids[i], TY_GROUP, &info);
                assert(ret == 0);
                assert(info.gid == ids[i]);
                printf("GROUP(id): %s(%u) --> ", info.group, info.gid);
                int j = 0;
                for (; info.group_list[j] != NULL; j++) {
			printf("%s, ", info.group_list[j]);
                }
                if (j != 0)
			printf("\n");
        }
	free_group_list(info.group_list);

        for (i = 0; idx_list[i] != 0; i++) {
		memset(&info, 0, sizeof(group_info));
		ret = query_by_idx(idx_list[i], TY_GROUP, &info);
                assert(ret == 0);
                printf("GROUP(idx): %s(%u) --> ", info.group, info.gid);
		int j = 0;
		for (; info.group_list[j] != NULL; j++) {
			printf("%s, ", info.group_list[j]);
		}
		if (j != 0)
			printf("\n");
        }
        free_group_list(info.group_list);
}

static void test_shadow_query() {
	int i = 0;
	int ret = 0;
	shadow_info info;
	char *names[] = {"wtest1", "wtest2", NULL};
        unsigned int idx_list[] = {1, 2, 0};

        for (; names[i] != NULL; i++) {
		memset(&info, 0, sizeof(shadow_info));
		ret = query(names[i], TY_SHADOW, &info);
                assert(ret == 0);
		assert(strcmp(info.user, names[i]) == 0);
                printf("SHADOW: %s -> %s\n", info.user, info.passwd);
	}

        for (i = 0; idx_list[i] != 0; i++) {
		memset(&info, 0, sizeof(shadow_info));
		ret = query_by_idx(idx_list[i], TY_SHADOW, &info);
                assert(ret == 0);
                printf("SHADOW(idx): %s -> %s\n", info.user, info.passwd);
        }
}

static void test_message()
{
	int ret = 0;
	char data[] = "001:0:1:test1:5000:5000:/home/test1:/bin/shell";
        response_info info = {
            .version = "001",
            .code = 0,
            .type = 1,
            .data = "test1:5000:5000:/home/test1:/bin/shell"
        };

	int len = 0;
	char *msg = NULL;
	ret = marshal_message(&info, &msg, &len);
        assert(ret == 0);
        assert(strcmp(msg, data) == 0);
	free(msg);
	
	response_info tmp;
	ret = unmarshal_message(data, &tmp);
        assert(ret == 0);
	assert(strcmp(tmp.version, info.version) == 0);
	assert(tmp.code == info.code);
	assert(tmp.type == info.type);
	assert(strcmp(tmp.data, info.data) == 0);
	free(tmp.data);
}

int main()
{
	test_passwd_query();
	test_group_query();
	test_shadow_query();

	test_message();
	
	return 0;
}
