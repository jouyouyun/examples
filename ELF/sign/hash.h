#ifndef __HASH_H__
#define __HASH_H__

#include <stdio.h>

char* sha256_data(char* data, int data_size, size_t *digest_size);

#endif
