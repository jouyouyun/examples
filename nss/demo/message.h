#pragma once

#include "common.h"

#define REQ_TY_QUERY_UID 1
#define REQ_TY_QUERY_USERNAME 1
#define REQ_TY_QUERY_GID 1
#define REQ_TY_QUERY_GROUPNAME 1

typedef struct _request_info {
	char version[4];
	unsigned int type;
	union value {
		unsigned int id;
		char name[MAX_NAME_LEN];
	};
} request_info;

typedef struct _response_info {
	char version[4];
	unsigned int code;
	unsigned int type;
	char *data; // need free
} response_info;

int marshal_message(response_info *info, char **data, int *len);
int unmarshal_message(char *data, response_info *info);
