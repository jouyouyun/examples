/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * client.c -- sd-bus client example
 *
 * DBus Service Info:
 *     Bus Type: user
 *     Service: com.deepin.daemon.Appearance
 *     Path: /com/deepin/daemon/Appearance
 *     Interface: com.deepin.daemon.Appearance
 *     Method: Show(s, as) s
 *
 * busctl: busctl --user call com.deepin.daemon.Appearance \
 *         /com/deepin/daemon/Appearance \
 *         com.deepin.daemon.Appearance Show "sas" "gtk" 1 "deepin"
 *
 * Written on 星期一, 22 四月 2019.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <systemd/sd-bus.h>

#define DBUS_DEST "com.deepin.daemon.Appearance"
#define DBUS_PATH "/com/deepin/daemon/Appearance"
#define DBUS_IFC "com.deepin.daemon.Appearance"
#define DBUS_METHOD "Show"

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <theme type> <theme name>\n", argv[0]);
        fprintf(stderr, "\tTheme available type: gtk, icon, cursor\n");
        return 0;
    }

    static sd_bus *bus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    char *detail = NULL;
    int ret = 0;

    ret = sd_bus_open_user(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to open session bus: %s\n", strerror(errno));
        return -1;
    }

    ret = sd_bus_call_method(bus, DBUS_DEST, DBUS_PATH, DBUS_IFC, DBUS_METHOD,
                       &error, &reply, "sas", argv[1], 1, argv[2]);
    if (ret < 0) {
        fprintf(stderr, "Failed to call method: (%s --> %s)", error.name, error.message);
        goto out;
    }

    ret = sd_bus_message_read(reply, "s", &detail);
    if (ret < 0) {
        fprintf(stderr, "Failed to read reply: %s\n", strerror(errno));
        goto out;
    }

    printf("Response: %s\n", detail);

out:
    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    return ret > 0? 0: 1;
}
