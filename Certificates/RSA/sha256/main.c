#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

#include "wosign.h"

static char *sha256_hash(const char *data, const size_t size, int hex);
static char *hexify(const unsigned char *data, const size_t size);
static int do_encrypt(const char *key_file, const char *data, const size_t size,
					   char **store, size_t *store_size);
static int do_decrypt(const char *cert_file, const char *data, const size_t size,
					  const char *sig, const size_t sig_size);
static int do_verify(const char *cert_file, const char *data, const size_t size,
					  const char *sig, const size_t sig_size);
static RSA *get_cert_key(const char *filename, int pub);
static RSA *get_bio_key(const char *filename);
static char *map_file(const char *filename, size_t *size);

static void
test_rsa(const char *key_file, const char *cert_file, const char *data)
{
	int ret = 0;
	char *store = NULL;
	size_t len = 0;

	printf("Will rsa encrypt\n");
	ret = do_encrypt(key_file, data, strlen(data), &store, &len);
	if (ret != 0) {
		return ;
	}

	printf("Will rsa decrypt\n");
	ret = do_decrypt(cert_file, data, strlen(data), store, len);
	if (ret != 1) {
		free(store);
		return;
	}

	free(store);
	store = NULL;
	printf("RSA Success\n");
}

static void
test_wosign(const char *cert_file, const char *data)
{
	int ret = 0;
	char *store = NULL;
	size_t len = 0;
	wosign_sign_config config = {
		.pin = strdup("88888888"),
		.container = strdup("C62A12CB-4C7B-4DE3-9E7E-7D3AF4843EEE"),
		.id = strdup("C62A12CB-4C7B-4DE3-9E7E-7D3AF4843EEE#2"),
		.algorithm = 0x40UL,
	};

	printf("\nWill wosign encrypt\n");
	ret = wosign_config_init(&config);
	if (ret != 0) {
		fprintf(stderr, "failed to init config\n");
		return ;
	}

	ret = wosign_sign(data, strlen(data), &store, &len);
	if (ret != 0) {
		fprintf(stderr, "failed to wosign sign\n");
		return;
	}

	printf("Will wosign decrypt\n");
	do_verify(cert_file, data, strlen(data), store, len);

	ret = do_decrypt(cert_file, data, strlen(data), store, len);
	if (ret != 1) {
		free(store);
		return;
	}


	free(store);
	store = NULL;
	printf("WoSign Success\n");
}

int
main(int argc, char *argv[])
{
	if (argc == 4) {
		test_rsa(argv[1], argv[2], argv[3]);
	} else if (argc == 3) {
		test_wosign(argv[1], argv[2]);
	} else {
		fprintf(stderr, "Usage: %s [<key file>] <cert file> <data>\n", argv[0]);
	}

	return 0;
}

static int
do_encrypt(const char *key_file, const char *data, const size_t size,
					   char **store, size_t *store_size)
{
	int ret = 0;
	int buf_size = 1 << 8;
	char *digest = NULL;
	char *buf = NULL;
	RSA *rsa = NULL;

	digest = sha256_hash(data, size, 1);
	if (!digest) {
		fprintf(stderr, "failed to hash data\n");
		return -11;
	}
	printf("\tAfter sha256: %s\n", digest);

	rsa = get_cert_key(key_file, 0);
	if (!rsa) {
		ret = -12;
		goto free;
	}

	buf = (char*)calloc(buf_size, sizeof(char));
	if (!buf) {
		fprintf(stderr, "failed to alloc memory for private encrypt\n");
		ret = -14;
		goto free;
	}

	buf_size = RSA_private_encrypt(strlen(digest), (unsigned char*)digest,
								   (unsigned char*)buf, rsa, RSA_PKCS1_PADDING);
	if (buf_size == -1) {
		fprintf(stderr, "failed to private encrypt\n");
		ret = -15;
		free(buf);
		buf = NULL;
		goto free;
	}

	*store = buf;
	*store_size = buf_size;

free:
	free(digest);
	digest = NULL;

	return ret;
}

static int
do_decrypt(const char *cert_file, const char *data, const size_t size,
				const char *sig, const size_t sig_size)
{
	int ret = 0;
	int len = 0;
	char buf[1<<10] = {0};
	char *digest = NULL;
	RSA *rsa = NULL;

	digest = sha256_hash(data, size, 1);
	if (!digest) {
		fprintf(stderr, "[%s] failed to sha256\n", __func__);
		return 0;
	}
	printf("\tAfter sha256: '%s', %ld\n", digest, strlen(digest));

	rsa = get_bio_key(cert_file);
	if (!rsa) {
		free(digest);
		return 0;
	}

	memset(buf, 0, 1<<10);
	len = RSA_public_decrypt((int)sig_size, (unsigned char*)sig,
							  (unsigned char*)buf, rsa, RSA_PKCS1_PADDING);
	RSA_free(rsa);

	printf("[%s]\tresult: %d, %ld\n", __func__, len, strlen(buf));
	ret = ((int)(strlen(digest)) == len && (memcmp(digest, buf, len) == 0));
	free(digest);

	return ret;
}

static int
do_verify(const char *cert_file, const char *data, const size_t size,
					 const char *sig, const size_t sig_size)
{
	int ret = 0;
	char *digest = NULL;
	RSA *rsa = NULL;

	digest = sha256_hash(data, size, 0);
	if (!digest) {
		fprintf(stderr, "[%s] failed to sha256\n", __func__);
		return -1;
	}

	printf("\tAfter sha256: '%s', %ld\n", digest, strlen(digest));
	printf("%d, %d, %d\n", sig[0], sig[1], sig[2]);
	rsa = get_bio_key(cert_file);
	ret = RSA_verify(NID_sha256, (unsigned char*)digest, strlen(digest),
					 (unsigned char*)sig, sig_size, rsa);
	if (ret != 1) {
		fprintf(stderr, "[%s]\tfailed to do verify: %d\n", __func__, ret);
		ret = -1;
	}

	if (ret == 1) {
		ret = 0;
		printf("[%s]\t--Success--\n", __func__);
	}
	free(digest);
	digest = NULL;
	RSA_free(rsa);
	return ret;
}

static RSA*
get_cert_key(const char *filename, int pub)
{
	FILE *fp = NULL;
	RSA *rsa = NULL;

	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "failed to open key file\n");
		return NULL;
	}

	if (pub) {
		rsa = PEM_read_RSAPublicKey(fp, NULL, NULL, NULL);
	} else {
		rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
	}
	fclose(fp);
	fp = NULL;
	if (!rsa) {
		fprintf(stderr, "failed to read %s key\n", pub?"public":"private");
	}

	return rsa;
}

static RSA*
get_bio_key(const char *filename)
{
	BIO *c = NULL;
	X509 *x509 = NULL;
	EVP_PKEY *pubkey = NULL;
	RSA *rsa = NULL;
	char *data = NULL;
	size_t len = 0;

	data = map_file(filename, &len);
	if (!data) {
		fprintf(stderr, "failed to mmap cert file\n");
		return NULL;
	}

	c = BIO_new(BIO_s_mem());
	BIO_puts(c, data);
	x509 = PEM_read_bio_X509(c, NULL, NULL, NULL);
	if (!x509) {
		fprintf(stderr, "failed to read x509\n");
		goto bio;
	}
	pubkey = X509_get_pubkey(x509);
	rsa = EVP_PKEY_get1_RSA(pubkey);

	X509_free(x509);
	EVP_PKEY_free(pubkey);

bio:
	BIO_free(c);
	c = NULL;
	munmap(data, len);
	data = NULL;

	return rsa;
}

static char*
sha256_hash(const char *data, const size_t size, int hex)
{
	SHA256_CTX ctx;
	unsigned char digest[SHA256_DIGEST_LENGTH] = {0};
	char *ret = (char*)calloc(SHA256_DIGEST_LENGTH+1, sizeof(char));

	SHA256_Init(&ctx);
	SHA256_Update(&ctx, data, size);
	memset(digest, 0, SHA256_DIGEST_LENGTH);
	SHA256_Final(digest, &ctx);

	if (!hex) {
		memcpy(ret, digest, SHA256_DIGEST_LENGTH);
		return ret;
	}
	free(ret);
	return hexify(digest, SHA256_DIGEST_LENGTH);
}

static char*
hexify(const unsigned char *digest, const size_t size)
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

static char*
map_file(const char *filename, size_t *size)
{
	int fd = 0;
	struct stat st = {0};
	char *cache_addr = NULL;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "error opening %s: %m\n", filename);
		return 0;
	}

	if (fstat(fd, &st) == -1) {
		fprintf(stderr, "error fstat %s: %m\n", filename);
		goto out;
	}

	cache_addr = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (cache_addr == MAP_FAILED) {
		fprintf(stderr, "error mmap %s: %m\n", filename);
		goto out;
	}

	*size = st.st_size;

out:
	close(fd);
	return cache_addr;
}
