#include "pack_nss.h"

/**
 * buffer: store result data
 * data: a /etc/group line
 *      format: <name>:<passwd>:<gid>:<members>
 **/
int pack_group(struct group *result, char *buffer, int buflen, char *data)
{
	int i = 0;
	int ret = 0;
	gint len = 0;
	gint mem_len = 0;
	int leftlen = buflen;
	char *cur = buffer;
	gchar **list = NULL;
	gchar **members = NULL;

	list = g_strsplit(data, DELIM, -1);
	if (!list)
		return errno;

	if (g_strv_length(list) != 4)
		goto_out(ret);
	
	memset(buffer, 0, buflen);
	len = strlen(list[0]);
	str_valid(len, ret);
	if (leftlen < len + 1)
		goto_out(ret);
	memcpy(cur, list[0], len);
	result->gr_name = cur;
	cur += len + 1;
	leftlen -= (len + 1);

        len = strlen(list[1]);
        str_valid(len, ret);
        if (leftlen < len + 1)
		goto_out(ret);
        memcpy(cur, list[1], len);
	result->gr_passwd = cur;
        cur += len + 1;
        leftlen -= (len + 1);

	result->gr_gid = atoi(list[2]);
	if (result->gr_gid < 1)
		goto_out(ret);

	if (strlen(list[3]) == 0)
		goto out;
	
	members = g_strsplit(list[3], ",", -1);
	mem_len = g_strv_length(members);
	result->gr_mem = (char**)cur;
        leftlen -= (sizeof(char *) * mem_len);
	if (leftlen < 1) {
		g_strfreev(members);
		goto_out(ret);
	}
        cur += sizeof(char *) * mem_len;

	for (; i < mem_len; i++) {
		len = strlen(members[i]);
		if (leftlen < len+1) {
			ret = -1;
			break;
		}
		memcpy(cur, members[i], len);
		result->gr_mem[i] = cur;
		cur += len + 1;
                leftlen -= (len + 1);
        }
        g_strfreev(members);
	
out:
	g_strfreev(list);
	return ret;
}
