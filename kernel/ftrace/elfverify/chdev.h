#pragma once

#define VERIFY_OK	0
#define VERIFY_FAIL	1
#define VERIFY_NOMEM	2
#define VERIFY_INTR	3
#define VERIFY_UNKNOWN	4

int verify_elf(const char* root, const char* path,
	       unsigned int major, unsigned int minor);
int register_elf_verifier_dev(void);
void unregister_elf_verifier_dev(void);
