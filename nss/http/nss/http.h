#pragma once

#include <nss.h>

#include "pack_nss.h"

#define IDX_MAX 1020

char *query_by_id(const gint id, gint *errno);
char *query_by_name(const gchar *name, gint *errno);

#define nss_setent(_func_, _idx_)				\
	enum nss_status _nss_http_set##_func_##ent(void)	\
	// TODO(jouyouyun): load uid list
	{ _idx_ = 1; return NSS_STATUS_SUCCESS; }
#define nss_endent(_func_)				\
	enum nss_status _nss_http_end##_func_##ent(void)	\
	{ return NSS_STATUS_SUCCESS; }
#define nss_getent_r(_func_, _idx_, _ret_ty_, _pack_) \
	enum nss_status _nss_http_get##_func_##ent_r(_ret_ty_ *result, \
						     char *buffer, size_t buflen, int *errnop) \
	{\
		int ret = 0;					\
		char *data = NULL;				\
		if (_idx_ > IDX_MAX) {				\
			*errnop = ENOENT;			\
			return NSS_STATUS_NOTFOUND;		\
		}						\
		data = query_by_id(_idx_, &ret);		\
		if (ret != 0) {					\
			*errnop = ENOENT;			\
			return NSS_STATUS_NOTFOUND;		\
		}						\
		_idx_++;					\
		ret = _pack_(result, buffer, buflen, data);	\
		g_free(data);					\
		if (ret != 0) {					\
			*errnop = ENOENT;			\
			return NSS_STATUS_NOTFOUND;		\
		}						\
		return NSS_STATUS_SUCCESS;			\
	}
