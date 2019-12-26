#pragma once

#include <pkcs11/cryptoki.h>
#include <stddef.h>

#define MAX_SM2_SIGN_LEN 2048

enum {
	InitError = 1,
	LoginError,
	LogoutError,
	GenKeyPairError,
	SignError,
	VerifyError,
	RemoveKeyPairError,
	InitConfigError,
};

typedef struct _wosign_sign_config_ {
	char* container;
	char* id;
	char* pin;
	unsigned long algorithm;
}wosign_sign_config;

int wosign_config_init(wosign_sign_config *config);
int wosign_sign(const char* data, const size_t len,
	     char** signature, size_t* sig_len);
