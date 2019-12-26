#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
		return 0;
	}

	struct stat stat;

	int ret = lstat(argv[1], &stat);
	if (ret != 0) {
		fprintf(stderr, "Failed to lstat: %s\n", strerror(errno));
		return -1;
	}

	printf("Is symlink: %d\n", S_ISLNK(stat.st_mode));

	return 0;
}
