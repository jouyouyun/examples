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

static int
match_callback(sd_bus_message *m, void *userdata, sd_bus_error *error)
{
	int ret = 0;
	int sum = 0;

        const char *sender = sd_bus_message_get_sender(m);
        const char *dest = sd_bus_message_get_destination(m);
        const char *path = sd_bus_message_get_path(m);
        const char *ifc = sd_bus_message_get_interface(m);
        const char *member = sd_bus_message_get_member(m);

        ret = sd_bus_message_read(m, "i", &sum);
	if (ret < 0) {
		fprintf(stderr, "Failed to read message: %s\n", strerror(-ret));
		return ret;
	}

	printf("Match triggered! sender='%s', dest='%s',\npath='%s', interface='%s',\nmember='%s', sum=%d\n",
	       sender, dest, path, ifc, member, sum);
	return 0;
}

int
main(int argc, char *argv[])
{
	int ret = 0;
	int32_t x, y, sum;
	int timeout = 0;
	sd_bus *bus = NULL;
	//sd_bus_message *m = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <x> <y> <timeout>\n", argv[0]);
		return 0;
	}
	x = (int32_t)atoi(argv[1]);
        y = (int32_t)atoi(argv[2]);
	timeout = atoi(argv[3]);

        ret = sd_bus_open_user(&bus);
	if (ret < 0) {
		fprintf(stderr, "Failed to open user bus: %s\n", strerror(-ret));
		return -1;
	}

        ret = sd_bus_match_signal(bus, NULL, BUS_DEST, BUS_PATH,
                                  BUS_IFC, "Result", match_callback, NULL);
        if (ret < 0)
		fprintf(stderr, "Failed to match signal: %s\n", strerror(-ret));

        /*
        ret = sd_bus_call_method(bus, BUS_DEST, BUS_PATH, BUS_IFC,
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
	*/

        sd_bus_message *_m = NULL;
        while (1) {
		ret = sd_bus_process(bus, &_m);
		if (ret < 0) {
			fprintf(stderr, "Failed to process: %s\n", strerror(-ret));
			break;
		}
		if (ret > 0)
			continue;
		ret = sd_bus_wait(bus, (uint64_t)-1);
                if (ret < 0) {
			fprintf(stderr, "Failed to wait: %s\n", strerror(-ret));
			break;
                }
        }
        sd_bus_message_unref(_m);

        //printf("Sum: %d\n", sum);
finish:
	sd_bus_error_free(&error);
	//sd_bus_message_unref(m);
	sd_bus_unref(bus);
	
	return ret < 0?-1:0;
}
