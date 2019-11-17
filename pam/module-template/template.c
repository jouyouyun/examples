/**
 * Description: pam 模块模板
 *
 * Compile: gcc -Wall -g -fPIC -shared -o pam_template.so  template.c
 *
 **/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <syslog.h>
#include <security/pam_ext.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>

#define MAX_BUF_SIZE 1024

/**
 * 发送信息给调用者
 * style 可为:
 *     PAM_TEXT_INFO: 打印一般的文本信息
 *     PAM_ERROR_MSG: 打印错误信息
 *     PAM_PROMPT_ECHO_ON: 提示用户，响应回显
 *     PAM_PROMPT_ECHO_OFF: 提示用户，禁止响应回显
 **/
static int send_message(pam_handle_t *pamh, const char *msg, int style);

/**
 * 认证管理接口，用于用户认证
 **/
PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return send_message(pamh, "Success", PAM_TEXT_INFO);
}

/**
 * 证书设置接口
 **/
PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return 0;
}

/**
 * 口令管理接口，用于设置口令
 **/
PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return 0;
}

/**
 * 账号管理接口，用于修改用户信息
 **/
PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return 0;
}

static int
send_message(pam_handle_t *pamh, const char *msg, int style)
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
    if (ret != PAM_SUCCESS)
        return -1;

    if (!pconv || !pconv->conv)
        return -1;

    ret = pconv->conv(1, &pmsg_ptr, &presp, pconv->appdata_ptr);
    if (ret != PAM_SUCCESS)
        return -1;

    return 0;
}
