#include "sm4.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRYPT_MODE_ECB "ecb"
#define CRYPT_MODE_CBC "cbc"

#define MODE_ENCRYPT "encrypt"
#define MODE_DECRYPT "decrypt"

extern void print_bytes(const char *msg, const uint8_t *data, int len);

static int str_to_ints(const char *str, int len, uint8_t *key) {
  char tmp[4] = {0};
  int i = 0, n = 0;

  tmp[0] = '0';
  tmp[1] = 'x';
  memset(key, 0, BLOCK_SIZE);
  for (; i < len; i++) {
    memcpy(tmp + 2, str + (i * 2), 2);
    n = strtol(tmp, NULL, 16);
    if (errno == ERANGE)
      return -errno;

    key[i] = (int8_t)(n)&0xFF;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  uint8_t key[BLOCK_SIZE] = {0};
  uint8_t iv[BLOCK_SIZE] = {0};
  uint8_t *input = NULL;
  uint8_t *data = NULL;
  int mode = SM4_ENCRYPT;
  int ret = 0, len = 0;

  if (argc < 5) {
    fprintf(stderr,
            "Usage: %s <block mode> <encrypt mode> <key> <iv> "
            "<cleartext/ciphertext>\n",
            argv[0]);
    fprintf(stderr, "Notice: the key, iv, data must be hexadecimal string with "
                    "strip '0x', length >= 32 and length mod 16 == 0\n");
    fprintf(stderr, "Such as:\n");
    fprintf(stderr, "\tkey   : 0123456789ABCDEFFEDCBA9876543210\n");
    fprintf(stderr, "\tplain : 0123456789ABCDEFFEDCBA9876543210\n");
    fprintf(stderr, "\tcipher: 681EDF34D206965E86b3E94F536E4246\n");
    return -1;
  }

  if (strcmp(argv[2], MODE_DECRYPT) == 0) {
    mode = SM4_DECRYPT;
  }

  ret = str_to_ints(argv[3], BLOCK_SIZE, key);
  if (ret < 0)
    return ret;

  print_bytes("key", key, BLOCK_SIZE);

  if (strcmp(argv[1], CRYPT_MODE_ECB) == 0) {
    goto ecb;
  } else if (strcmp(argv[1], CRYPT_MODE_CBC) == 0) {
    goto cbc;
  } else {
    return -1;
  }

ecb:
  len = strlen(argv[4]);
  input = calloc(len / 2, sizeof(uint8_t));
  if (!input) {
    fprintf(stderr, "alloc memory: %s\n", strerror(errno));
    return -errno;
  }

  ret = str_to_ints(argv[4], len / 2, input);
  if (ret < 0) {
    free(input);
    return ret;
  }
  
  if (mode == SM4_ENCRYPT)
    data = sm4_ecb_encrypt((uint8_t *)key, input, len / 2);
  else
    data = sm4_ecb_decrypt((uint8_t *)key, input, len / 2);

  goto out;

cbc:
  ret = str_to_ints(argv[4], BLOCK_SIZE, iv);
  if (ret)
    return ret;

  len = strlen(argv[5]);
  input = calloc(len / 2, sizeof(uint8_t));
  if (!input) {
    fprintf(stderr, "alloc memory: %s\n", strerror(errno));
    return -errno;
  }

  ret = str_to_ints(argv[5], len / 2, input);
  if (ret < 0) {
    free(input);
    return ret;
  }

  if (mode == SM4_ENCRYPT)
    data = sm4_cbc_encrypt(key, iv, input, len / 2);
  else
    data = sm4_cbc_decrypt(key, iv, input, len / 2);

  goto out;

out:
  print_bytes("input", input, len / 2);
  free(input);
  
  if (!data)
    return -1;

  print_bytes("output", data, len / 2);
  free(data);

  return 0;
}
