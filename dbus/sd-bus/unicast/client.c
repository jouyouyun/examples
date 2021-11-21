/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * client.c -- A dbus client by sd-bus.
 *
 * Written on 星期五, 19 十一月 2021.
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <systemd/sd-bus.h>

#include "common.h"

int
main(int argc, char *argv[])
{
	int ret = 0;
	int32_t x, y, sum;
	sd_bus *bus = NULL;
	sd_bus_message *m = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <x> <y>\n", argv[0]);
		return 0;
	}
	x = (int32_t)atoi(argv[1]);
        y = (int32_t)atoi(argv[2]);

        ret = sd_bus_open_user(&bus);
	if (ret < 0) {
		fprintf(stderr, "Failed to open user bus: %s\n", strerror(-ret));
		return -1;
	}

        ret = sd_bus_call_method(bus, SRV_BUS_DEST, SRV_BUS_PATH, SRV_BUS_DEST,
                                 "Add", &error, &m, "ii", x, y);
        if (ret < 0) {
                fprintf(stderr, "Failed to call method: %s\n", strerror(-ret));
                goto finish;
        }

        ret = sd_bus_message_read(m, "i", &sum);
        if (ret < 0) {
                fprintf(stderr, "Failed to read message: %s\n", strerror(-ret));
                goto finish;
        }

        printf("Sum: %d\n", sum);
finish:
	sd_bus_error_free(&error);
	sd_bus_message_unref(m);
	sd_bus_unref(bus);
	
	return ret < 0?-1:0;
}
