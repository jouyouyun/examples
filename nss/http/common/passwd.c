#include "pack_nss.h"

/**
 * buffer: store result data
 * data: a /etc/passwd line
 *      format: <name>:<passwd>:<uid>:<gid>:<gecos>:<home>:<shell>
 **/
int pack_passwd(struct passwd *result, char *buffer, int buflen, char *data)
{
	int ret = 0;
	gint len = 0;
	char *cur = buffer;
	gchar **list = NULL;

	if (buflen < strlen(data) + 5)
		return -1;
	
	list = g_strsplit(data, DELIM, -1);
	if (!list)
		return errno;

	if (g_strv_length(list) != 7) {
		goto_out(ret);
	}
	
	memset(buffer, 0, buflen);
	len = strlen(list[0]);
	str_valid(len, ret);
	memcpy(cur, list[0], len);
	result->pw_name = cur;
	cur += len + 1;

	len = strlen(list[1]);
	memcpy(cur, list[1], len);
	result->pw_passwd = cur;
	cur += len + 1;

	result->pw_uid = atoi(list[2]);
	result->pw_gid = atoi(list[3]);
	if (result->pw_uid < 1 || result->pw_gid < 0) {
		goto_out(ret);
        }

        len = strlen(list[4]);
	memcpy(cur, list[4], len);
	result->pw_gecos = cur;
	cur += len + 1;

	len = strlen(list[5]);
        str_valid(len, ret);
        memcpy(cur, list[5], len);
        result->pw_dir = cur;
        cur += len + 1;

        len = strlen(list[6]);
        str_valid(len, ret);
        memcpy(cur, list[6], len);
        result->pw_shell = cur;

out:
	g_strfreev(list);
	list = NULL;
        return ret;
}
