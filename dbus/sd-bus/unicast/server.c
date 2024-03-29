/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * server.c -- A dbus service generated by sd-bus
 *
 * Compile: $ gcc server.c -o server `pkg-config --cflags --libs libsystemd`
 *
 * Written on 星期五, 19 十一月 2021.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <systemd/sd-bus.h>
#include <systemd/sd-bus-vtable.h>

#include "common.h"

static int signal_result(sd_bus_message *m, int32_t sum)
{
	int ret = 0;
	sd_bus_creds *creds = NULL;
        sd_bus_message *signal = NULL;
        pid_t pid = 0;
        uid_t uid = 0;
	uid_t owner_uid = 0;
	const char *comm = NULL;
	const char *exe = NULL;
	const char *desc = NULL;

        const char *sender = sd_bus_message_get_sender(m);
        const char *dest = sd_bus_message_get_destination(m);
	const char *path = sd_bus_message_get_path(m);
	const char *ifc = sd_bus_message_get_interface(m);
	const char *member = sd_bus_message_get_member(m);

	printf("Sender='%s', dest='%s',\npath='%s', interface='%s',\nmember='%s'\n",
	       sender, dest, path, ifc, member);

	creds = sd_bus_message_get_creds(m);
	sd_bus_creds_get_pid(creds, &pid);
	sd_bus_creds_get_uid(creds, &uid);
	sd_bus_creds_get_owner_uid(creds, &owner_uid);
	sd_bus_creds_get_comm(creds, &comm);
	sd_bus_creds_get_exe(creds, &exe);
	sd_bus_creds_get_description(creds, &desc);

	printf("uid: %x, pid: %x, owner: %x,\ncomm: '%s', exe: '%s', desc: '%s'\n",
	       uid, pid, owner_uid, comm, exe, desc);

	ret = sd_bus_message_new_signal(sd_bus_message_get_bus(m), &signal,
					SRV_BUS_PATH, SRV_BUS_DEST, "Result");
	if (ret < 0) {
		fprintf(stderr, "Failed to new signal message: %s\n", strerror(-ret));
		return ret;
	}

	sd_bus_message_set_destination(signal, LISTEN_BUS_DEST);
	sd_bus_message_append(signal, "i", sum);
        sd_bus_send(sd_bus_message_get_bus(m), signal, NULL);
	
	return 0;
}

static int method_add(sd_bus_message *m, void *userdata, sd_bus_error *error)
{
	int32_t x = 0, y = 0;
	int ret = 0;

	ret = sd_bus_message_read(m, "ii", &x, &y);
	if (ret < 0) {
		fprintf(stderr, "Failed to read message: %s\n", strerror(-ret));
		return ret;
	}

	signal_result(m, x + y);
	return sd_bus_reply_method_return(m, "i", x + y);
}

static sd_bus *bus = NULL;
static sd_bus_slot *slot = NULL;
static const sd_bus_vtable vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("Add", "ii", "i", method_add, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_SIGNAL("Result", "i", 0),
	SD_BUS_VTABLE_END
};

int
main(int argc, char *argv[])
{
	int ret = 0;

	ret = sd_bus_open_user(&bus);
	if (ret < 0) {
		fprintf(stderr, "Failed to open user bus: %s\n", strerror(-ret));
		return ret;
	}

	ret = sd_bus_add_object_vtable(bus, &slot, SRV_BUS_PATH, SRV_BUS_DEST, vtable, NULL);
	if (ret < 0) {
		fprintf(stderr, "Failed to add object: %s\n", strerror(-ret));
                goto finish;
        }

        ret = sd_bus_request_name(bus, SRV_BUS_DEST, 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to own name: %s\n", strerror(-ret));
		goto finish;
        }

	while(1) {
		ret = sd_bus_process(bus, NULL);
		if (ret < 0) {
			fprintf(stderr, "Failed to process bus: %s\n", strerror(-ret));
			goto finish;
		}
		if (ret > 0)
			continue;

		ret = sd_bus_wait(bus, (uint64_t)-1);
                if (ret < 0) {
			fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-ret));
			goto finish;
                }
        }

 finish:
	sd_bus_slot_unref(slot);
	sd_bus_unref(bus);
	return ret < 0?-1:0;
}
