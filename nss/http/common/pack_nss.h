#pragma once

#include <glib.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>

#define DELIM ":"
#define MAX_BUF_LEN 4096

int pack_passwd(struct passwd *result, char *buffer, int buflen, char *data);
int pack_group(struct group *result, char *buffer, int buflen, char *data);
int pack_shadow(struct spwd *result, char *buffer, int buflen, char *data);

#define goto_out(__ret__) {			\
		__ret__ = -1;			\
		goto out;			\
	}

#define str_valid(_len_, _ret_)			\
	if (_len_ < 1)				\
		goto_out(_ret_)			\
