#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define PAM_SM_AUTH
#include <security/pam_modules.h>

#include "common.h"

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc,
		    const char **argv)
{
	int ret = 0;
	shadow_info info;
	const char *user = NULL;
	const char *token = NULL;

	ret = pam_get_user(pamh, &user, NULL);
	if (ret != PAM_SUCCESS) {
		WEN_INFO("failed to get username: %s", strerror(errno));
		return PAM_AUTH_ERR;
	}

	ret = query(user, TY_SHADOW, &info);
	if (ret != 0) {
		WEN_INFO("failed to query shadow: %s", strerror(ret));
		return  PAM_AUTH_ERR;
	}

	ret = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&token);
	if (ret != PAM_SUCCESS) {
		WEN_INFO("failed to get token for %s: %s", user, strerror(errno));
		return PAM_AUTH_ERR;
	}
	
	if ((strcmp(info.user, user) == 0) && (strcmp(info.passwd, "") == 0)) {
		passwd_info u_info;
		ret = query(user, TY_PASSWD, &u_info);
		if (ret == 0) {
			char env_user[1024] = {0};
			char env_home[1024] = {0};
			char env_shell[1024] = {0};
			memset(env_user, 0, 1024);
			memset(env_home, 0, 1024);
			memset(env_shell, 0, 1024);
			sprintf(env_user, "USER=%s", u_info.user);
			sprintf(env_home, "HOME=%s", u_info.home);
			sprintf(env_shell, "SHELL=%s", u_info.shell);
			pam_putenv(pamh, env_user);
			pam_putenv(pamh, env_home);
			pam_putenv(pamh, env_shell);
			setenv("USER", u_info.user, 1);
			setenv("HOME", u_info.home, 1);
			setenv("SHELL", u_info.shell, 1);
		}
		return PAM_SUCCESS;
	}

	WEN_INFO("failed to compare token for %s", user);
	return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc,
                              const char **argv)
{
  return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc,
                                const char **argv)
{
	return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc,
                                const char **argv) {
	return PAM_IGNORE;
}
