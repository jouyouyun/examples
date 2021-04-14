#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "message.h"

extern char **split(const char *str, const char *delim, int *size);

int marshal_message(response_info *info, char **data, int *len) {
	if (info->code > 999 || info->type > 99)
		return -1;

	*len = 8 + strlen(info->data) + 4;
	*data = (char *)calloc(9, sizeof(char) * (*len));
	sprintf(*data, "%s:%u:%u:%s", info->version, info->code, info->type,
		info->data);

	return 0;
}

int unmarshal_message(char *data, response_info *info)
{
	int ret = 0;
	int size = 0;
	int i = 0, len = 0;
	char **strv = NULL;

	strv = split(data, ":", &size);
	if (!strv)
		return -1;

	if (size < 5) {
		free(strv);
		return -2;
	}

	memset(info, 0, sizeof(response_info));
	memcpy(info->version, strv[0], 3);
	info->code = strtoul(strv[1], NULL, 10);
	info->type = strtoul(strv[2], NULL, 10);

	for (i = 3; i < size; i++) {
		len += strlen(strv[i]);
	}

	info->data = (char*)calloc(len+(size-4)+1, sizeof(char));
	memcpy(info->data, strv[3], strlen(strv[3]));
	char *tmp = info->data + strlen(strv[3]);
	for (i = 4; i < size; i++) {
		sprintf(tmp, ":%s", strv[i]);
		tmp += strlen(strv[i]) + 1;
	}
	
	return ret;
}
