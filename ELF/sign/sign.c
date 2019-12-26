#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <gelf.h>

#include "hash.h"

#define SECTION_PROG_PROTECT ".my_prog_protect"

typedef struct {
	int fd;
	Elf *elf;
} elf_wrapper;

static elf_wrapper *create_elf_wrapper(const char *filename);
static void free_elf_wrapper(elf_wrapper *ew);

#define GET_EHDR(hehdr) do {\
	if (!gelf_getehdr(_ew->elf, &ehdr)) { \
		fprintf(stderr, "Failed to get ehdr\n");	\
	}												\
	} while(0);

#define GET_SHDR(shdr) do {\
	} while(0);

static elf_wrapper *_ew = NULL;

static char*
mmap_wrapper(const char *filename, size_t *size)
{
	int fd = 0;
	struct stat stat;
	char *addr = NULL;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &stat) == -1) {
		fprintf(stderr, "Failed to stat %s: %s\n", filename, strerror(errno));
		goto err_out;
	}

	addr = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		fprintf(stderr, "Failed to mmap %s: %s\n", filename, strerror(errno));
		goto err_out;
	}

	*size = stat.st_size;
err_out:
	close(fd);
	return addr;
}

static elf_wrapper*
create_elf_wrapper(const char *filename)
{
	elf_wrapper *ew = (elf_wrapper*)calloc(1, sizeof(elf_wrapper));
	if (!ew) {
		return NULL;
	}

	ew->fd = open(filename, O_RDWR);
	if (ew->fd == -1) {
		fprintf(stderr, "Failed to open file: %s\n", filename);
		goto fails;
	}
	printf("Open '%s': %d\n", filename, ew->fd);

	Elf *elf = elf_begin(ew->fd, ELF_C_RDWR_MMAP, NULL);
	if (!elf) {
		fprintf(stderr, "Failed to process elf\n");
		goto fails;
	}

	ew->elf = elf;

	// TODO(jouyouyun): support more elf kind, such as: ELF_K_AR
	if (elf_kind(ew->elf) != ELF_K_ELF) {
		fprintf(stderr, "Not an elf file: %s\n", filename);
		goto fails;
	}

	return ew;

fails:
	free_elf_wrapper(ew);
	return NULL;
}

static void
free_elf_wrapper(elf_wrapper *ew)
{
	if (!ew) {
		return;
	}

	if (ew->fd > 0) {
		close(ew->fd);
	}
	if (ew->elf) {
		elf_end(ew->elf);
		ew->elf = NULL;
	}
	free(ew);
	ew = NULL;
}

/* static size_t */
/* remove_elf_section(char *content, size_t fsize, const char *section) */
/* { */
/*      GElf_Ehdr *ehdr = NULL; */
/*      GElf_Shdr *shdr = NULL; */
/*      GElf_Shdr *section_hdr = NULL; */
/*      char *shdr_tab = NULL; */
/*      char *section_ptr = NULL; */
/*      char *section_name_ptr = NULL; */
/*      char *shname = NULL; */
/*      size_t size = 0; */
/*      size_t shdr_tabsize = 0; */
/*      size_t section_size = 0; */
/*      int found = 0; */
/*      int i = 0; */

/*      ehdr = (GElf_Ehdr*)content; */
/*      shdr = (GElf_Shdr*)(content+ehdr->e_shoff); */
/*      shdr_tabsize = shdr[ehdr->e_shstrndx].sh_size; */
/*      shdr_tab = (char*)(content + shdr[ehdr->e_shentsize].sh_offset); */

/*      for (; i < ehdr->e_shnum; i++) { */
/* 	  shname = shdr_tab + shdr[i].sh_name; */
/* 	  if (strcmp(shname, section) != 0) */
/* 	       continue; */
/* 	  section_ptr = (char*)(shdr+i); */
/* 	  section_name_ptr = shname; */
/* 	  section_size = shdr[i].sh_size; */
/* 	  found = 1; */
/* 	  section_hdr = shdr + i; */
/* 	  section_hdr->sh_name -= (strlen(section) +1); */
/*      } */

/*      if (!found) { */
/* 	  fprintf(stderr, "Not found section: %s\n", section); */
/* 	  return 0; */
/*      } */

/*      return size; */
/* } */


void elf_remove_section(char* elf_fcontent, size_t elf_fsize,
						char* section_name,
						size_t* new_elf_fsize)
{
	// No remove, just move data, it'll eventually cover all the unwanted data.
	//
	// 1. Move shstrtab before shstrtab entry.
	// 2. Move shstrtab after shstrtab entry.
	// 3. Move data between shstrtab and section headers before target section
	//    header entry.
	// 4. Move data after target section header entry till the end of the file.

	GElf_Ehdr ehdr;
	GElf_Shdr *shdrs = NULL;
	size_t shstrtab_size = 0;

	size_t section_data_size = 0;
	char* section_hdr_ptr = 0;
	char* section_name_entry_ptr = 0;
	int found = 0;

	if (!gelf_getehdr(_ew->elf, &ehdr)) {
		fprintf(stderr, "Failed to get ehdr\n");
		return ;
	}

	shdrs = (GElf_Shdr*)(elf_fcontent + ehdr.e_shoff);
	shstrtab_size = shdrs[ehdr.e_shstrndx].sh_size;

	char* shstrtab = (char*)(elf_fcontent + shdrs[ehdr.e_shstrndx].sh_offset);
	for (int i = 0; i < ehdr.e_shnum; i++) {
		char *shname = shstrtab + shdrs[i].sh_name;
		if (strcmp(shname, section_name) == 0) {
			section_hdr_ptr = (char*)(shdrs + i);
			section_name_entry_ptr = shname;
			section_data_size = shdrs[i].sh_size;
			found = 1;
			continue;
		}
		if (found) {
			// update name offset of sections after the target section.
			GElf_Shdr* shdr = shdrs + i;
			shdr->sh_name -= (strlen(section_name) + 1);
		}
	}

	if (!found) {
		fprintf(stderr, "section %s not found\n", section_name);
		return;
	}

	char* data_ptr = 0;
	size_t data_size = 0;
	size_t move_offset = 0;

	// Move shstrtab before shstrtab entry.
	data_ptr = shstrtab;
	data_size = section_name_entry_ptr - shstrtab;
	move_offset = section_data_size;
	memmove(data_ptr - move_offset, data_ptr, data_size);
	GElf_Shdr* shstrtab_shdr = (GElf_Shdr*)(shdrs + ehdr.e_shstrndx);
	shstrtab_shdr->sh_offset -= move_offset;
	shstrtab_shdr->sh_size -= (strlen(section_name) + 1);
	// End

	// Move shstrtab after shstrtab entry.
	data_ptr = section_name_entry_ptr + strlen(section_name) + 1;
	data_size = shstrtab + shstrtab_size - data_ptr;
	move_offset += strlen(section_name) + 1;
	memmove(data_ptr - move_offset, data_ptr, data_size);
	// End


	// Move data between shstrtab and section headers before target section header entry.
	data_ptr = shstrtab + shstrtab_size;
	data_size = section_hdr_ptr - data_ptr;
	memmove(data_ptr - move_offset, data_ptr, data_size);
	ehdr.e_shoff -= move_offset;
	ehdr.e_shnum -= 1;
	// End


	// Move data after target section header entry till the end of the file.
	data_ptr = section_hdr_ptr + sizeof(GElf_Shdr);
	data_size = elf_fcontent + elf_fsize - data_ptr;
	move_offset += sizeof(GElf_Shdr);
	memmove(data_ptr - move_offset, data_ptr, data_size);
	// End

	*new_elf_fsize = elf_fsize
		- section_data_size
		- sizeof(GElf_Shdr)
		- (strlen(section_name) + 1);
}


int elf_append_section(char* elf_fcontent,
					   size_t elf_fsize,
					   char* section_name,
					   char* section_data,
					   size_t section_data_size,
					   char** new_elf_fcontent,
					   size_t* new_elf_fsize)
{
	*new_elf_fsize = elf_fsize
		+ section_data_size /* section data */
		+ sizeof(GElf_Shdr) /* section header */
		+ (strlen(section_name) + 1) /* section name shstrtab entry*/;
	*new_elf_fcontent = realloc(elf_fcontent, *new_elf_fsize);
	if ((*new_elf_fcontent) == NULL) {
		fprintf(stderr, "failed to realloc: %s\n", strerror(errno));
		return -1;
	}

	GElf_Ehdr ehdr;
	GElf_Shdr* shdrs = NULL;
	char* shstrtab = NULL;

	if (!gelf_getehdr(_ew->elf, &ehdr)) {
		fprintf(stderr, "Failed to get ehdr\n");
		return -1;
	}

	shdrs = (GElf_Shdr*)(new_elf_fcontent + ehdr.e_shoff);
	shstrtab = (char*)(new_elf_fcontent + shdrs[ehdr.e_shstrndx].sh_offset);

	size_t shdrs_size = ehdr.e_shnum * ehdr.e_shentsize;
	size_t shstrtab_size = shdrs[ehdr.e_shstrndx].sh_size;

	// 1. Move data after section headers, may exists, see libwidevinecdm.so.
	// 2. Insert section header before data (merged in section headers).
	// 3. Move section headers and data between shstrtab and section headers.
	// 4. Insert shstrtab entry before that.
	// 5. Move shstrtab.
	// 6. Insert section data before shstrtab.

	char* data_ptr = 0;
	size_t data_size = 0;
	size_t move_offset = 0;

	// Begin moving data after section headers.
	data_ptr = (char*)shdrs + shdrs_size;
	data_size = *new_elf_fcontent + elf_fsize - data_ptr;
	move_offset = section_data_size + sizeof(GElf_Shdr) + (strlen(section_name) + 1);
	memmove(data_ptr + move_offset, data_ptr, data_size);
	data_ptr += move_offset;
	// End.


	// Begin insert section header
	GElf_Shdr section_header = {0};
	// sh_name and sh_offset filled later.
	section_header.sh_type = SHT_PROGBITS;
	section_header.sh_size = section_data_size;
	memcpy(data_ptr - sizeof(GElf_Shdr), &section_header, sizeof(GElf_Shdr));
	ehdr.e_shnum++;
	// End


	// Begin moving section headers and data between shstrtab and section headers.
	data_ptr = shstrtab + shstrtab_size;
	data_size = (char*)shdrs + shdrs_size - data_ptr;
	move_offset = section_data_size + strlen(section_name) + 1;
	memmove(data_ptr + move_offset, data_ptr, data_size);
	data_ptr += move_offset;
	ehdr.e_shoff += move_offset;
	shdrs = (GElf_Shdr*)((char*)shdrs + move_offset);
	// End

	// Begin inserting section name shstrtab entry
	char* name_entry = data_ptr - (strlen(section_name) + 1);
	strcpy(name_entry, section_name);
	// End

	// Begin moving shstrtab.
	data_ptr = shstrtab;
	data_size = shstrtab_size;
	move_offset = section_data_size;
	memmove(data_ptr + move_offset, data_ptr, data_size);
	data_ptr += move_offset;
	shstrtab += move_offset;
	GElf_Shdr* shstrtab_sh = shdrs + ehdr.e_shstrndx;
	shstrtab_sh->sh_offset += move_offset;
	shstrtab_sh->sh_size += (strlen(section_name) + 1);
	// End

	// Begin inserting signature section content.
	char* section_data_addr = data_ptr - section_data_size;
	memcpy(section_data_addr, section_data, section_data_size);
	GElf_Shdr* our_sh = shdrs + (ehdr.e_shnum - 1);
	our_sh->sh_name = name_entry - shstrtab;
	// TODO(hualet): other arches?
	our_sh->sh_offset = (Elf64_Off)(section_data_addr - *new_elf_fcontent);
	// End

	return 0;
}

int elf_get_code_segment(char* elf_addr, char** elf_seg_addr, size_t* elf_seg_size)
{
	GElf_Ehdr ehdr;
	GElf_Phdr *phdrs = NULL;
	size_t offset = 0, size = 0;
	int i = 0;

	if (!gelf_getehdr(_ew->elf, &ehdr)) {
		fprintf(stderr, "Failed to get ehdr\n");
		return -1;
	}

	phdrs = (GElf_Phdr*)(elf_addr + ehdr.e_phoff);

	for (; i < ehdr.e_phnum; i++) {
		if ((phdrs[i].p_flags & PF_X) == 0 || phdrs[i].p_type != PT_LOAD)
			continue;

		offset = phdrs[i].p_offset;
		size = phdrs[i].p_filesz;

		if (offset < sizeof(GElf_Ehdr)) {
			size = size + offset - sizeof(GElf_Ehdr);
			offset = sizeof(GElf_Ehdr);
		}
		break;
	}

	*elf_seg_size = size;
	*elf_seg_addr = elf_addr + offset;

	if (*elf_seg_size == 0)
		return 0;

	return 1;
}

static int elf_calculate_signature(char* elf_fcontent, char** signature, size_t* signature_size)
{
	char *digest = NULL;
	char* elf_seg_addr = NULL;
	size_t elf_seg_size = 0;

	int ok = elf_get_code_segment(elf_fcontent, &elf_seg_addr, &elf_seg_size);
	if (!ok) {
		fprintf(stderr, "failed to get ELF code segment.\n");
		return 0;
	}

	digest = sha256_data((char *)elf_seg_addr, elf_seg_size, signature_size);
	*signature = digest;

	return 1;
}

static void debug_print_section_data(char *content, int offset, int size)
{
	char *data = NULL;

	if (size == 0) {
		fprintf(stderr, "\tno data in section\n");
		return;
	}

	data = (char*)malloc(size+1);
	if (!data) {
		fprintf(stderr, "\tfailed to malloc\n");
		return;
	}
	memset(data, 0, size+1);
	memcpy(data, content + offset, size);
	data[size] = '\0';
	fprintf(stderr, "\tdata: %s", data);
	free(data);
}

static void debug_print_elf(char* elf_fcontent)
{
	GElf_Ehdr ehdr;
	GElf_Shdr* shdrs = NULL;

	if (!gelf_getehdr(_ew->elf, &ehdr)) {
		fprintf(stderr, "Failed to get ehdr\n");
		return ;
	}

	shdrs = (GElf_Shdr*)(elf_fcontent + ehdr.e_shoff);
	char* shstrtab = (char*)(elf_fcontent + shdrs[ehdr.e_shstrndx].sh_offset);
	for (int i = 0; i < ehdr.e_shnum; i++) {
		char *shname = shstrtab + shdrs[i].sh_name;
		fprintf(stderr, "header name: %s %d\n", shname, shdrs[i].sh_name);
		debug_print_section_data(elf_fcontent, shdrs[i].sh_offset, shdrs[i].sh_size);
	}
}

static int write_origin_elf(const char* data, size_t data_size, const char* fn)
{
	int fd = open(fn, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "error opening %s: %m\n", fn);
		return 0;
	}

	size_t wrote = write(fd, data, data_size);
	if (wrote != data_size) {
		fprintf(stderr, "failed to dump data to file");
		close(fd);
		return 0;
	}

	close(fd);
	return 1;
}

int
main(int argc, char *argv[])
{
	elf_wrapper *ew = NULL;
	char *content = NULL;
	char *content_tmp = NULL;
	char *content_sign = NULL;
	size_t fsize = 0, fsize_tmp = 0, fsize_sign = 0;
	int elfclass = 0;
	int ret = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <elf filename>\n", argv[0]);
		return -1;
	}

	/* Tell the library which version we are expecting.  */
	(void) elf_version (EV_CURRENT);

	ew = create_elf_wrapper(argv[1]);
	if (!ew) {
		fprintf(stderr, "Failed to create elf wrapper\n");
		return -1;
	}

	elfclass = gelf_getclass(ew->elf);
	if (elfclass == ELFCLASS32) {
		fprintf(stderr, "32 bit\n");
	} else if (elfclass == ELFCLASS64) {
		fprintf(stderr, "64 bit\n");
	} else {
		fprintf(stderr, "Invalid elf class: %d\n", elfclass);
		free_elf_wrapper(ew);
		return -1;
	}

	content = (char*)mmap_wrapper(argv[1], &fsize);
	if (!content) {
		fprintf(stderr, "Failed to mmap file\n");
		free_elf_wrapper(ew);
		return -1;
	}

	content_tmp = (char*)malloc(fsize);
	if (!content_tmp) {
		munmap(content, fsize);
		free_elf_wrapper(ew);
		fprintf(stderr, "Failed to alloc content: %s\n", strerror(errno));
		return -1;
	}

	memset(content_tmp, 0, fsize);
	memcpy(content_tmp, content, fsize);
	munmap(content, fsize);

	fsize_tmp = fsize;
	_ew = ew;
	elf_remove_section(content_tmp, fsize, SECTION_PROG_PROTECT, &fsize_tmp);
	fprintf(stderr, "Content 1: %lu, 2: %lu\n", fsize, fsize_tmp);
	elf_calculate_signature(content_tmp, &content_sign, &fsize_sign);
	fprintf(stderr, "Sign text: %s, %lu\n", content_sign, fsize_sign);
	ret = elf_append_section(content_tmp, fsize_tmp, SECTION_PROG_PROTECT,
							 content_sign, fsize_sign, &content_tmp, &fsize_tmp);
	free(content_sign);
	if (ret == -1) {
		free_elf_wrapper(ew);
		free(content_tmp);
		return -1;
	}

	debug_print_elf(content_tmp);

	write_origin_elf(content_tmp, fsize_tmp, argv[1]);
	free_elf_wrapper(ew);
	free(content_tmp);
	return 0;
}
