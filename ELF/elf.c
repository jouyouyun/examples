#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gelf.h>

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <elf file>\n", argv[0]);
		return 0;
	}

	/* Tell the library which version we are expecting.  */
	(void) elf_version (EV_CURRENT);

	int fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open file\n");
		return -1;
	}

	Elf *elf = elf_begin(fd, ELF_C_RDWR_MMAP, NULL);
	if (!elf) {
		fprintf(stderr, "Failed to parse elf\n");
		close(fd);
		return -1;
	}

	int kind = elf_kind(elf);
	switch (kind) {
	case ELF_K_ELF:
		printf("ELF File\n");
		break;
	case ELF_K_NONE:
		printf("None\n");
		break;
	case ELF_K_AR:
		printf("ELF AR\n");
		break;
	}

	elf_end(elf);
	close(fd);

	return 0;
}
