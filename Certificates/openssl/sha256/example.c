// gcc -Wall -g example.c -lcrypto
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

static char*
hexify(const unsigned char *digest, ssize_t size)
{
	int i = 0;
	static char fmt[] = "0123456789abcdef";
	char *buf = (char*)calloc(size * 2 + 1, sizeof(char));

	for (; i < size; i++) {
		buf[i*2] = fmt[(digest[i]>>4) & 0xf];
		buf[i*2+1] = fmt[digest[i] & 0xf];
	}

	buf[size*2] = '\0';
	return buf;
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <data>\n", argv[0]);
		return 0;
	}

	int i = 0;
	SHA256_CTX sc;
	unsigned char digest[SHA256_DIGEST_LENGTH] = {0};
	char *value = NULL;

	SHA256_Init(&sc);
	SHA256_Update(&sc, argv[1], strlen(argv[1]));

	memset(digest, 0, SHA256_DIGEST_LENGTH);
	SHA256_Final(digest, &sc);

	printf("Digest: %ld--%d\n\t", strlen((char*)digest), SHA256_DIGEST_LENGTH);
	for (; i < SHA256_DIGEST_LENGTH; i++) {
		printf("<%c-%d-%d>", digest[i], (int)digest[i], i);
	}
	printf("\n\n");

	value = hexify(digest, SHA256_DIGEST_LENGTH);
	if (!value) {
		return -1;
	}

	printf("Hash: %s\n", value);
	free(value);
	return 0;
}
