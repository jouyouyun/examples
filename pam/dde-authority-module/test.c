/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * pam.c -- dde-authority pam module
 *
 * Compile: gcc -Wall -g -fPIC -shared pam.c -o dde_authority_pam.so \
 *          `pkg-config --cflags --libs libsystemd`
 *
 * Written on 星期二, 23 四月 2019.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <systemd/sd-bus.h>

#define DBUS_DEST "com.deepin.daemon.Authority"
#define DBUS_PATH "/com/deepin/daemon/Authority"
#define DBUS_IFC "com.deepin.daemon.Authority"
#define DBUS_M_HAS_COOKIE "HasCookie"
#define DBUS_M_CHECK_COOKIE "CheckCookie"

static int has_cookie(sd_bus *bus, const char *username);
static int check_cookie(sd_bus *bus, const char *username, const char *cookie);

int main(int argc, const char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <username> <cookie>\n", argv[0]);
    }

    sd_bus *bus = NULL;
    int ret = 0;

    ret = sd_bus_open_system(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to open system bus: %s\n",
                 strerror(errno));
        return -1;
    }

    ret = has_cookie(bus, argv[1]);
    if (ret != 1) {
        goto finish;
    }

    ret = check_cookie(bus, argv[1], argv[2]);
    if (ret != 1) {
        goto finish;
    }

finish:
    sd_bus_unref(bus);

    return (ret == 1) ? 0 : -1;
}

static int
has_cookie(sd_bus *bus, const char *username)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    int ret = 0;
    int result = 0;

    ret = sd_bus_call_method(bus, DBUS_DEST, DBUS_PATH, DBUS_IFC,
                             DBUS_M_HAS_COOKIE, &err, &reply, "s", username);
    if (ret < 0) {
        fprintf(stderr, "Failed to call 'HasCookie': %s, %s\n",
                 err.name, err.message);
        goto finish;
    }

    ret = sd_bus_message_read(reply, "b", &result);
    if (ret < 0) {
        fprintf(stderr, "Failed to read 'HasCookie' value: %s\n",
                 strerror(errno));
        goto finish;
    }

    fprintf(stderr, "[DEBUG] HasCookie value: %d\n", result);

finish:
    sd_bus_error_free(&err);
    sd_bus_message_unref(reply);

    return ret < 0 ? 0 : 1;
}

static int
check_cookie(sd_bus *bus, const char *username, const char *cookie)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    int ret = 0;
    int result = 0;

    fprintf(stderr, "[DEBUG] CheckCookie cookie: %s\n", cookie);

    ret = sd_bus_call_method(bus, DBUS_DEST, DBUS_PATH, DBUS_IFC,
                             DBUS_M_CHECK_COOKIE, &err, &reply,
                             "ss", username, cookie);
    if (ret < 0) {
        fprintf(stderr, "Failed to call 'CheckCookie': %s, %s\n",
                 err.name, err.message);
        goto finish;
    }

    ret = sd_bus_message_read(reply, "b", &result);
    if (ret < 0) {
        fprintf(stderr, "Failed to read 'CheckCookie' value: %s\n",
                 strerror(errno));
        goto finish;
    }

    fprintf(stderr, "[DEBUG] CheckCookie value: %d\n", result);

finish:
    sd_bus_error_free(&err);
    sd_bus_message_unref(reply);

    return ret < 0 ? 0 : 1;
}
