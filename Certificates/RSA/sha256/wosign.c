#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "global.h"
#include "wosign.h"

static CK_RV wosign_init(void);
static CK_RV wosign_destory(void);
static CK_RV wosign_login(void);
static CK_RV wosign_logout(void);
static CK_RV wosign_find_key_pai(void);
static char *wosign_do_sign(const char *data, const int len, size_t *sign_len);
static void free_conifg(wosign_sign_config *config);

static struct _wosign_info_ {
	CK_SESSION_HANDLE hSession;
	CK_SLOT_ID_PTR pSlotList;
	CK_OBJECT_HANDLE hPubKey;
	CK_OBJECT_HANDLE hPriKey;
	CK_MECHANISM mechanism;

	wosign_sign_config config;
}_sign_info;


// algorithm, such as: CKK_RSA, CKM_SM2_SM3_SIGN_VERIFY
// static CK_MECHANISM ckMechanism = { CKM_SM2_SM3_SIGN_VERIFY, NULL_PTR, 0 };

int wosign_config_init(wosign_sign_config *config)
{
	if (!config || !config->pin || !config->container || !config->id) {
		return -1;
	}

	_sign_info.config.pin = config->pin;
	_sign_info.config.container = config->container;
	_sign_info.config.id = config->id;
	_sign_info.mechanism.mechanism = config->algorithm;
	_sign_info.mechanism.pParameter = NULL;
	_sign_info.mechanism.ulParameterLen = 0;

	return 0;
}

int wosign_sign(const char* data, const size_t len, char** signature, size_t* sig_len)
{
	if (data == 0 || len == 0)
		return -1;

	int ret = 0;
	CK_RV rv;
	char *tmp = NULL;

	rv = wosign_init();
	if (rv != CKR_OK) {
		ret = -InitError;
		goto destroy;
	}
	se_msg(LOG_DEBUG,"Init [Success]\n");

	rv = wosign_login();
	if (rv != CKR_OK) {
		ret = -LoginError;
		goto destroy;
	}
	se_msg(LOG_DEBUG,"Login [Success]\n");

	rv = wosign_find_key_pai();
	if (rv != CKR_OK) {
		ret = -GenKeyPairError;
		goto logout;
	}
	se_msg(LOG_DEBUG,"Find key [Success]\n");

	tmp = wosign_do_sign(data, len, sig_len);
	if (!tmp) {
		ret = -SignError;
		goto logout;
	}
	se_msg(LOG_DEBUG,"Signature [Success]\n");

	rv = wosign_logout();
	if (rv != CKR_OK) {
		ret = -LogoutError;
		goto free;
	}
	se_msg(LOG_DEBUG,"Logout [Success]\n");

	ret = 0;
	*signature = tmp;
	goto destroy;

logout:
	wosign_logout();
free:
	free(tmp);
	tmp = NULL;
destroy:
	wosign_destory();
	return ret;
}

static CK_RV wosign_init(void)
{
	if (!_sign_info.config.pin) {
		return -InitConfigError;
	}

	// init _sign_info
	_sign_info.hSession = 0;
	_sign_info.hPriKey = 0;
	_sign_info.hPubKey = 0;
	_sign_info.pSlotList = NULL;

	CK_RV rv = C_Initialize(NULL_PTR);
	if (CKR_OK != rv)
	 {
		 return rv;
	 }

	CK_ULONG ulCount = 0;
	rv = C_GetSlotList(TRUE, NULL_PTR, &ulCount);
	if (CKR_OK != rv)
	 {
		 return rv;
	 }
	if (ulCount <= 0)
	 {
		 rv = CKR_GENERAL_ERROR;
		 return rv;
	 }

	_sign_info.pSlotList = (CK_SLOT_ID_PTR)calloc(ulCount, sizeof(CK_SLOT_ID));
	if (!_sign_info.pSlotList)
	 {
		 return rv;
	 }

	rv = C_GetSlotList(TRUE, _sign_info.pSlotList, &ulCount);
	if (CKR_OK != rv)
	 {
		 return rv;
	 }

	CK_TOKEN_INFO tokenInfo;
	rv = C_GetTokenInfo(_sign_info.pSlotList[0], &tokenInfo);
	if (CKR_OK != rv)
	 {
		 return rv;
	 }

	if (tokenInfo.flags&CKF_USER_PIN_LOCKED)
	 {
		 return CKR_GENERAL_ERROR;
	 }

	rv = C_OpenSession(_sign_info.pSlotList[0],
			   CKF_RW_SESSION | CKF_SERIAL_SESSION,
			   NULL_PTR, NULL_PTR, &_sign_info.hSession);
	if (CKR_OK != rv)
	 {
		 return rv;
	 }

	return CKR_OK;
}

static CK_RV wosign_destory(void)
{
	free_conifg(&_sign_info.config);

	if (NULL_PTR != _sign_info.pSlotList) {
		free(_sign_info.pSlotList);
		_sign_info.pSlotList = NULL_PTR;
	}

	CK_RV rv = CKR_OK;
	if (_sign_info.hSession) {
		rv = C_CloseSession(_sign_info.hSession);
	}
	rv = C_Finalize(0);

	return rv;
}

static void free_conifg(wosign_sign_config *config)
{
	if (!config) {
		return;
	}

	if (config->pin) {
		free(config->pin);
		config->pin = NULL;
	}
	if (config->container) {
		free(config->container);
		config->container = NULL;
	}
	if (config->id) {
		free(config->id);
		config->id = NULL;
	}
}

static CK_RV wosign_login(void)
{
	CK_RV rv;

	rv = C_Login(_sign_info.hSession, CKU_USER, (unsigned char*)_sign_info.config.pin, strlen((char *)_sign_info.config.pin));
	if (rv != CKR_OK)
	 {
		 return rv;
	 }

	return CKR_OK;
}


static CK_RV wosign_logout(void)
{
	CK_RV rv;
	rv = C_Logout(_sign_info.hSession);
	if (rv != CKR_OK)
	 {
		 return rv;
	 }

	return CKR_OK;
}

static CK_RV wosign_find_key_pai(void) {
    //init
    _sign_info.hPubKey = (CK_OBJECT_HANDLE)0;
    _sign_info.hPriKey = (CK_OBJECT_HANDLE)0;

    CK_OBJECT_CLASS pubKeyClass = CKO_PUBLIC_KEY;
    CK_OBJECT_CLASS PriKeyClass = CKO_PRIVATE_KEY;
    unsigned char IsToken=1;

    CK_ATTRIBUTE pubkeyTempl[] =
    {
        { CKA_CLASS, &pubKeyClass, sizeof(CKO_PUBLIC_KEY)},
        { CKA_TOKEN, &IsToken, sizeof(char)},
		{ CKA_ID, _sign_info.config.id, strlen(_sign_info.config.id) },
		{ CKA_CONTAINER,_sign_info.config.container,strlen(_sign_info.config.container) },
    };

    //find public key
    C_FindObjectsInit(_sign_info.hSession, pubkeyTempl, sizeof(pubkeyTempl)/sizeof(CK_ATTRIBUTE));

#define MAX_HANDLE_COUNT 255
    CK_OBJECT_HANDLE hCKObj[MAX_HANDLE_COUNT];
    CK_ULONG ulRetCount = 0;
    CK_RV ckrv = 0;

    ckrv = C_FindObjects(_sign_info.hSession, hCKObj, MAX_HANDLE_COUNT, &ulRetCount);  //get the count of pubkey
    if(CKR_OK != ckrv || ulRetCount == 0) {
        se_msg(LOG_ERROR, "no public key found: 0x%lx!!\n", ckrv);
        return -13;
    }
    if(ulRetCount >= 1) {
        //only get first for demo
        _sign_info.hPubKey = hCKObj[0];
        se_msg(LOG_INFO, "\n%lu, public key object found.\n", ulRetCount);
    }

    //find private key
    CK_ATTRIBUTE prikeyTempl[] =
    {
        { CKA_CLASS, &PriKeyClass, sizeof(CKO_PRIVATE_KEY) },
        { CKA_TOKEN, &IsToken, sizeof(char) },
        { CKA_ID, _sign_info.config.id, strlen(_sign_info.config.id) },
        { CKA_CONTAINER,_sign_info.config.container,strlen(_sign_info.config.container) },
    };

    C_FindObjectsInit(_sign_info.hSession, prikeyTempl, sizeof(prikeyTempl)/sizeof(CK_ATTRIBUTE));

    ckrv = C_FindObjects(_sign_info.hSession, hCKObj, MAX_HANDLE_COUNT, &ulRetCount);
    if(CKR_OK != ckrv || ulRetCount == 0) {
        se_msg(LOG_ERROR, "no private key found: 0x%lx!!\n", ckrv);
        return -13;
    }

    if(ulRetCount >= 1) {
        _sign_info.hPriKey = hCKObj[0];
        se_msg(LOG_INFO, "\n%lu private key object found.\n", ulRetCount);
    }

    return CKR_OK;
}

static char *wosign_do_sign(const char *data, const int len, size_t *sign_len)
{
	CK_RV rv;
	char *sign = NULL;
	size_t tmp_len = MAX_WOSIGN_SIGN_LEN;

	rv = C_SignInit(_sign_info.hSession, &_sign_info.mechanism, _sign_info.hPriKey);
	if (CKR_OK != rv) {
		se_msg(LOG_ERROR, "failed to init wosign sign: %lu\n", rv);
		return NULL;
	}

	sign = (char*)calloc(MAX_WOSIGN_SIGN_LEN, sizeof(char));
	if (!sign) {
		return NULL;
	}

	rv = C_Sign(_sign_info.hSession, (unsigned char*)data, len,
		    (unsigned char*)(sign), &tmp_len);
	if (rv != CKR_OK) {
		se_msg(LOG_ERROR, "failed to wosign sign: %lu\n", rv);
		free(sign);
		sign = NULL;
		return NULL;
	}

	*sign_len = tmp_len;
	return sign;
}
