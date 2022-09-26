/**
 * Copyright (C) 2022 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.c -- Test libaudit to log general message
 *
 * compile: gcc -Wall -g main.c `pkg-config --cflags --libs audit`
 *
 * Written on 星期一, 26 九月 2022.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libaudit.h>

int
main(int argc, char *argv[])
{
	int ret = 0;
	int fd = 0;

	fd = audit_open();
	if (fd == -1) {
		fprintf(stderr, "failed to open audit: %s\n", strerror(errno));
		return -1;
	}

	ret = audit_log_user_message(fd, AUDIT_TRUSTED_APP,
								 "I'm launched, but will then exit.", NULL, NULL, NULL, 1);
	if (ret <= 0) {
		fprintf(stderr, "failed to log message: %s\n", strerror(errno));
		goto close;
	}

 close:
	audit_close(fd);
	return ret;
}
