#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>


static char* hexify(const unsigned char* digest, ssize_t sz, size_t *digest_size)
{
	static char fmt[] = "0123456789abcdef";
	*digest_size = sz*2+1;
	char* s = malloc(*digest_size);

	for (int i = 0; i < sz; i++) {
		s[i*2] = fmt[(digest[i] >> 4) & 0xf];
		s[i*2+1] = fmt[digest[i] & 0xf];
	}

	s[sz*2] = 0;
	return s;
}

char* sha256_data(char* data, int data_size, size_t *digest_size)
{
	unsigned char digest[SHA256_DIGEST_LENGTH] = {0};

	SHA256_CTX sc;
	SHA256_Init(&sc);
	SHA256_Update(&sc, data, data_size);
	SHA256_Final(digest, &sc);

	return hexify(digest, SHA256_DIGEST_LENGTH, digest_size);
}
