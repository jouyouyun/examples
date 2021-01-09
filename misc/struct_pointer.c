#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_R 1 << 0
#define FLAG_W 1 << 1
#define FLAG_D 1 << 2

struct foo1 {
	unsigned long id;
	unsigned int flag;
	char *value;
};

static char *files[] = {
	"/opt/dlp/data/app1/file1",
	"/opt/dlp/data/app2/file1",
	NULL,
};

#define DEFAULT_FLAG (FLAG_R|FLAG_W|FLAG_D)

static struct foo1 *make_foo1(const char *filename, unsigned int flag)
{
	struct foo1 *ret = NULL;
	int len = strlen(filename);

	ret = calloc(1, sizeof(struct foo1));
	if (!ret)
		return NULL;

	ret->value = calloc(len+1, sizeof(char));
	if (!ret->value)
		goto free;

	ret->id = (unsigned long)ret;
	memcpy(ret->value, filename, len);
	ret->flag = flag;

	return ret;
free:
	free(ret);
	return NULL;
}

static void free_foo1(struct foo1 *node)
{
	free(node->value);
	free(node);
	node = NULL;
}

int main()
{
	int i = 0;

	while (files[i] != NULL) {
		struct foo1 *node = make_foo1(files[i], DEFAULT_FLAG);
		if (!node) {
			printf("failed to make node for '%s'\n", files[i]);
			i++;
			continue;
		}

		printf("node addr: %p\n", node);
		printf("node value: 0x%lx, %u, %s\n", node->id, node->flag, node->value);
		free_foo1(node);
		printf("node free success\n");
		i++;
	}

	return 0;
}
