/**
 * Description: pam 模块模板
 *
 * Compile: gcc -Wall -g -fPIC -shared -o pam_template.so  template.c
 *
 * common-passwd: password       requisite       pam_template.so
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

#define WEAK_PASSWD_FILE "/etc/rainbowlib"

static int is_weak_passwd(const char *filename, const char *passwd);

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
	return 0;
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
	int exists = 0;
	const char *token = NULL;
	int ret = 0;
	char buf[MAX_BUF_SIZE] = {0};

	ret = pam_get_authtok(pamh, PAM_AUTHTOK, &token, NULL);
	if (ret != PAM_SUCCESS) {
		memset(buf, 0, MAX_BUF_SIZE);
		snprintf(buf, MAX_BUF_SIZE, "failed to get token");
		send_message(pamh, buf, PAM_ERROR_MSG);
		return PAM_AUTHTOK_ERR;
	}

	exists = is_weak_passwd(WEAK_PASSWD_FILE, token);
	if (exists) {
		send_message(pamh, "密码太简单", PAM_TEXT_INFO);
		return PAM_AUTHTOK_ERR;
	}

	return PAM_IGNORE;
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

static int
is_weak_passwd(const char *filename, const char *passwd)
{
	char buf[MAX_BUF_SIZE] = {0};
	FILE *fr = NULL;
	char *ret = NULL;
	int length = 0;
	int exists = 0;

	fr = fopen(filename, "r");
	if (!fr)
		return 0;

	while(!feof(fr)) {
		memset(buf, 0, MAX_BUF_SIZE);
		ret = fgets(buf, MAX_BUF_SIZE, fr);
		if (!ret)
			continue;

		length = strlen(buf);
		if (buf[length-1] == '\n')
			buf[length-1] = '\0';
		if (strcasecmp(buf, passwd) == 0) {
			exists = 1;
			break;
		}
	}

	fclose(fr);
	return exists;
}
