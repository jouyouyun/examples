#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <magic.h>

#include "filetype.h"

char *query_filetype(const char *filename)
{
	int ret = 0;
	int len = 0;
	magic_t cookie;
	const char *data = NULL;
	char *filetype = NULL;

	cookie = magic_open(MAGIC_MIME);

	if (magic_load(cookie, NULL) != 0)
		goto out;

	data = magic_file(cookie, filename);
	if (!data)
		goto out;

	len = strlen(data);
	filetype = calloc(len + 1, sizeof(char));
	if (!filetype)
		goto out;

	memcpy(filetype, data, len);
 out:
	magic_close(cookie);
	return filetype;
}
