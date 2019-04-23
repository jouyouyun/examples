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

#define PAM_SM_AUTH
#include <security/pam_modules.h>

#define MAX_BUF_SIZE 1024

#define DBUS_DEST "com.deepin.daemon.Authority"
#define DBUS_PATH "/com/deepin/daemon/Authority"
#define DBUS_IFC "com.deepin.daemon.Authority"
#define DBUS_M_HAS_COOKIE "HasCookie"
#define DBUS_M_CHECK_COOKIE "CheckCookie"

static int send_msg(pam_handle_t *pamh, const char *msg, int style);
static int has_cookie(pam_handle_t *pamh, sd_bus *bus, const char *username);
static int check_cookie(pam_handle_t *pamh, sd_bus *bus, const char *username);

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags,
                                   int argc, const char **argv)
{
    sd_bus *bus = NULL;
    const char *username = NULL;
    char msg[MAX_BUF_SIZE] = {0};
    int ret = 0;

    ret = pam_get_user(pamh, &username, NULL);
    if (ret != PAM_SUCCESS) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to get username: %s",
                 strerror(errno));
        send_msg(pamh, msg, PAM_ERROR_MSG);
    }

    // TODO(jouyouyun): remove debug info
    memset(msg, 0, MAX_BUF_SIZE);
    snprintf(msg, MAX_BUF_SIZE, "[DEBUG] Authenticate for: %s", username);
    send_msg(pamh, msg, PAM_TEXT_INFO);

    ret = sd_bus_open_system(&bus);
    if (ret < 0) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to open system bus: %s",
                 strerror(errno));
        send_msg(pamh, msg, PAM_ERROR_MSG);
        return -1;
    }

    ret = has_cookie(pamh, bus, username);
    if (ret != 1) {
        goto finish;
    }

    ret = check_cookie(pamh, bus, username);
    if (ret != 1) {
        goto finish;
    }

finish:
    sd_bus_unref(bus);

    return (ret == 1) ? PAM_SUCCESS : PAM_AUTH_ERR;
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh, int flags,
                              int argc, const char **argv)
{
    return PAM_IGNORE;
}

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh, int flags,
                                int argc, const char **argv)
{
    return PAM_IGNORE;
}

static int
send_msg(pam_handle_t *pamh, const char *msg, int style)
{
    const struct pam_message pmsg = {
        .msg = msg,
        .msg_style = style,
    };
    const struct pam_message *pmsg_ptr = &pmsg;
    const struct pam_conv *pconv = NULL;
    struct pam_response *presp = NULL;
    int ret = 0;

    ret = pam_get_item(pamh, PAM_CONV, (const void**)&pconv);
    if (ret != PAM_SUCCESS) {
        return -1;
    }

    if (!pconv || !pconv->conv) {
        return -1;
    }

    ret = pconv->conv(1, &pmsg_ptr, &presp, pconv->appdata_ptr);
    if (ret != PAM_SUCCESS) {
        return -1;
    }

    return 0;
}

static int
has_cookie(pam_handle_t *pamh, sd_bus *bus, const char *username)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    char msg[MAX_BUF_SIZE] = {0};
    int ret = 0;
    int result = 0;

    ret = sd_bus_call_method(bus, DBUS_DEST, DBUS_PATH, DBUS_IFC,
                             DBUS_M_HAS_COOKIE, &err, &reply, "s", username);
    if (ret < 0) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to call 'HasCookie': %s, %s",
                 err.name, err.message);
        send_msg(pamh, msg, PAM_ERROR_MSG);
        goto finish;
    }

    ret = sd_bus_message_read(reply, "b", &result);
    if (ret < 0) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to read 'HasCookie' value: %s",
                 strerror(errno));
        send_msg(pamh, msg, PAM_ERROR_MSG);
        goto finish;
    }

    // TODO(jouyouyun): remove debug info
    memset(msg, 0, MAX_BUF_SIZE);
    snprintf(msg, MAX_BUF_SIZE, "[DEBUG] HasCookie value: %d", result);
    send_msg(pamh, msg, PAM_TEXT_INFO);

finish:
    sd_bus_error_free(&err);
    sd_bus_message_unref(reply);

    return ret < 0 ? 0 : 1;
}

static int
check_cookie(pam_handle_t *pamh, sd_bus *bus, const char *username)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    const char *cookie = NULL;
    char msg[MAX_BUF_SIZE] = {0};
    int ret = 0;
    int result = 0;

    ret = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&cookie);
    if (ret != PAM_SUCCESS) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to get auth token: %s",
                 strerror(errno));
        send_msg(pamh, msg, PAM_ERROR_MSG);
        return -1;
    }

    // TODO(jouyouyun): remove debug info
    memset(msg, 0, MAX_BUF_SIZE);
    snprintf(msg, MAX_BUF_SIZE, "[DEBUG] CheckCookie cookie: %s", cookie);
    send_msg(pamh, msg, PAM_TEXT_INFO);

    ret = sd_bus_call_method(bus, DBUS_DEST, DBUS_PATH, DBUS_IFC,
                             DBUS_M_CHECK_COOKIE, &err, &reply,
                             "ss", username, cookie);
    if (ret < 0) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to call 'CheckCookie': %s, %s",
                 err.name, err.message);
        send_msg(pamh, msg, PAM_ERROR_MSG);
        goto finish;
    }

    ret = sd_bus_message_read(reply, "b", &result);
    if (ret < 0) {
        memset(msg, 0, MAX_BUF_SIZE);
        snprintf(msg, MAX_BUF_SIZE, "Failed to read 'CheckCookie' value: %s",
                 strerror(errno));
        send_msg(pamh, msg, PAM_ERROR_MSG);
        goto finish;
    }

    // TODO(jouyouyun): remove debug info
    memset(msg, 0, MAX_BUF_SIZE);
    snprintf(msg, MAX_BUF_SIZE, "[DEBUG] CheckCookie value: %d", result);
    send_msg(pamh, msg, PAM_TEXT_INFO);

finish:
    sd_bus_error_free(&err);
    sd_bus_message_unref(reply);

    return ret < 0 ? 0 : 1;
}
