/**
 * compile: gcc -shared -o libnss_wen.so.2 -Wl,-soname,libnss_wen.so.2 nss_wen.c -lsystemd
 **/
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <nss.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>

#include "common.h"

#define MAX_INDEX 10
unsigned int g_idx = 1;

#define make_passwd()							\
	{								\
		char *ptr = buffer;					\
		*errnop = 0;						\
		if (strlen(info.user) + strlen(info.home) + strlen(info.shell) + 5 > \
		    buflen) {						\
			*errnop = ERANGE;				\
			return NSS_STATUS_TRYAGAIN;			\
		}							\
		result->pw_uid = info.uid;				\
		result->pw_gid = info.gid;				\
		memset(buffer, 0, buflen);				\
		memcpy(ptr, info.user, strlen(info.user));		\
		result->pw_name = ptr;					\
		ptr += strlen(info.user) + 1;				\
		memcpy(ptr, info.home, strlen(info.home));		\
		result->pw_dir = ptr;				\
		ptr += strlen(info.home) + 1;				\
		memcpy(ptr, info.shell, strlen(info.shell));		\
		result->pw_shell = ptr;					\
		ptr += strlen(info.shell) + 1;				\
		result->pw_passwd = ptr;				\
		result->pw_gecos = ptr;					\
	}
#define make_group()							\
	{								\
		char *ptr = buffer;					\
		*errnop = 0;						\
		if (strlen(info.group) + 3 > buflen) {			\
			*errnop = ERANGE;				\
			return NSS_STATUS_TRYAGAIN;			\
		}							\
		result->gr_gid = info.gid;				\
		memset(buffer, 0, buflen);				\
		memcpy(ptr, info.group, strlen(info.group));		\
		result->gr_name = ptr;					\
		ptr += strlen(info.group) + 1;				\
		// TODO(jouyouyun): impl group passwd
		memcpy(ptr, "x", 1);					\
		result->gr_passwd = ptr;				\
		ptr += 1;						\
		result->gr_mem = (char**)ptr;				\
		result->gr_mem[0] = NULL;				\
	}
#define make_shadow()							\
	{								\
		char *ptr = buffer;					\
		*errnop = 0;						\
		if (strlen(info.user) + strlen(info.passwd) + 2 > buflen) { \
			*errnop = ERANGE;				\
			return NSS_STATUS_TRYAGAIN;			\
		}							\
		memset(buffer, 0, buflen);				\
		memcpy(ptr, info.user, strlen(info.user));		\
		result->sp_namp = ptr;					\
		ptr += strlen(info.user) + 1;				\
		memcpy(ptr, info.passwd, strlen(info.passwd));		\
		result->sp_pwdp = ptr;				\
		ptr += strlen(info.passwd) + 1;				\
		result->sp_max = 99999;					\
		result->sp_warn = 7;					\
		result->sp_expire = INT32_MAX;				\
	}
#define nss_setent(_func)				\
	enum nss_status _nss_wen_set##_func##ent(void)	\
	{ return g_idx = 1; NSS_STATUS_SUCCESS; }
#define nss_endent(_func)				\
	enum nss_status _nss_wen_end##_func##ent(void)	\
	{ return NSS_STATUS_SUCCESS; }
#define nss_getent_r(_func, _info_ty, _ty, _result_ty, _make)		\
	enum nss_status _nss_wen_get##_func##ent_r(_result_ty *result, char *buffer, \
						   size_t buflen, int *errnop) { \
		int ret = 0;						\
		_info_ty info;						\
		WEN_INFO("[%s] idx: %u\n", __func__, g_idx);		\
		if (g_idx > MAX_INDEX) {				\
			*errnop = ENOENT;				\
			return NSS_STATUS_NOTFOUND;			\
		}							\
		memset(&info, 0, sizeof(_info_ty));			\
		ret = query_by_idx(g_idx, _ty, &info);			\
		if (ret != 0) {						\
			*errnop = ENOENT;				\
			return NSS_STATUS_NOTFOUND;			\
		}							\
		g_idx++;						\
		_make;							\
		return NSS_STATUS_SUCCESS;				\
	}
#define nss_getnam_r(_func, _info_ty, _ty, _result_ty, _make)		\
	enum nss_status _nss_wen_get##_func##nam_r(const char *name,	\
						   _result_ty *result, char *buffer, \
						   size_t buflen, int *errnop) { \
		int ret = 0;						\
		_info_ty info;						\
		WEN_INFO("[%s] name: %s\n", __func__, name);		\
		memset(&info, 0, sizeof(_info_ty));			\
		ret = query(name, _ty, &info);				\
		if (ret != 0) {						\
			*errnop = ENOENT;				\
			return NSS_STATUS_NOTFOUND;			\
		}							\
		_make;							\
		return NSS_STATUS_SUCCESS;				\
	}
#define nss_getid_r(_func, _info_ty, _ty, _id_ty, _result_ty, _make)	\
	enum nss_status _nss_wen_get##_func##id_r(_id_ty id, _result_ty *result, \
						  char *buffer, size_t buflen, \
						  int *errnop) {	\
		int ret = 0;						\
		_info_ty info;						\
		WEN_INFO("[%s] id: %u", __func__, id);			\
		memset(&info, 0, sizeof(_info_ty));			\
		ret = query_by_id((unsigned int)id, _ty, &info);	\
		if (ret != 0) {						\
			*errnop = ENOENT;				\
			return NSS_STATUS_NOTFOUND;			\
		}							\
		_make;							\
		return NSS_STATUS_SUCCESS;				\
	}

// Init passwd file
nss_setent(pw);
// Clean passwd file
nss_endent(pw);
// Iterator passwd file
nss_getent_r(pw, passwd_info, TY_PASSWD, struct passwd, make_passwd());
// Find a user account by name
nss_getnam_r(pw, passwd_info, TY_PASSWD, struct passwd, make_passwd());
// Find a user account by uid
nss_getid_r(pwu, passwd_info, TY_PASSWD, uid_t, struct passwd, make_passwd());

nss_setent(gr);
nss_endent(gr);
nss_getent_r(gr, group_info, TY_GROUP, struct group, make_group());
nss_getnam_r(gr, group_info, TY_GROUP, struct group, make_group());
nss_getid_r(grg, group_info, TY_GROUP, gid_t, struct group, make_group());

nss_setent(sp);
nss_endent(sp);
nss_getent_r(sp, shadow_info, TY_SHADOW, struct spwd, make_shadow());
nss_getnam_r(sp, shadow_info, TY_SHADOW, struct spwd, make_shadow());
