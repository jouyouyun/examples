#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_BUF_SIZE (4096)

static char *read_file_content(const char *filename, size_t *real_size)
{
	FILE *fr = NULL;
	char *content = NULL, *tmp = NULL;
	size_t size = 0;

	fr = fopen(filename, "r");
	if (!fr) {
		fprintf(stderr, "Failed to open: %s\n", strerror(errno));
		return NULL;
	}

	while (!feof(fr)) {
		tmp = (char*)realloc(content, size + MAX_BUF_SIZE);
		if (!tmp) {
			free(content);
			content = NULL;
			break;
		}

		content = tmp;
		memset(content + size, 0, MAX_BUF_SIZE);
		*real_size = fread(content + size, sizeof(char), MAX_BUF_SIZE, fr);
		fprintf(stderr, "[%s] real: %lu, size: %lu\n", __func__, *real_size, size);
		if (*real_size == 0){
			free(content);
			content = NULL;
			break;
		}

		if (*real_size != 0)
			size += *real_size;
	}

	fclose(fr);
	return content;
}

// mp equal or contains root
static int mounted_at(const char *mp, const char *root)
{
	if (strcmp(mp, root) == 0)
		return 1;
	if (strlen(mp) > strlen(root) && strstr(mp, root) == mp && mp[strlen(root)] == '/')
		return 1;
	return 0;
}

static int is_special_mp(const char *mp)
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

static void parse_mountinfo(char *content)
{
	char mp[MAX_BUF_SIZE] = {0};
	char *line = content;
	unsigned int major = 0, minor = 0;

	while(sscanf(line, "%*d %*d %d:%d %*s %250s %*s %*s %*s %*s %*s %*s\n", &major, &minor, mp) == 3) {
		line = strchr(line, '\n') + 1;

		fprintf(stderr, "[%s] scan line: major(%u), minor(%u), mp: %s\n",
			__func__, major, minor, mp);
		if (is_special_mp(mp))
			continue;
		fprintf(stderr, "\tvalidity\n");
	}
}

int main(int argc, char *argv[])
{
	char *content = NULL;
	size_t size = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <mountinfo file>\n", argv[0]);
		return 0;
	}

	content = read_file_content(argv[1], &size);
	if (!content) {
		fprintf(stderr, "Failed to read file\n");
		return -1;
	}

	fprintf(stderr, "Read %lu bytes content\n", size);
	parse_mountinfo(content);
	free(content);

	return 0;
}
