/**
 * pam_deepin_privilege module
 * if not in developer mode, disable sudo/pkexec own privilege in
 * terminal.
 *
 * Compile: gcc -Wall -g -fPIC -shared deepin_privilegeexit
.c -o pam_deepin_privilege.so
 *
 * Usage: add 'auth   requisite       pam_deepin_privilege.so debug' to
 * '/etc/pam.d/sudo' the first line
 *
**/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#define UNUSED __attribute__((unused))

// argument
#define PAM_DEBUG_ARG 0x01

#define MAX_BUF_SIZE 1024
// developer mode file, format: '{"enabled": true}'
#define DEVELOPER_MODE_FILE "/var/lib/deepin/developer_mode.json"
#define DEVELOPER_MODE_ENABLE_STR "\"enabled\":true"
#define WHITELIST_FILE "/var/lib/deepin/deepin_privilege.whitelist"

static int parse_argument(const pam_handle_t *pamh, int argc, const char **argv);

static int has_in_developer_mode();
static int check_whitelist(pam_handle_t *pamh, const int debug);

static void
print_file(const pam_handle_t *pamh, const char *filename)
{
	FILE *fr = fopen(filename, "r");
	if (!fr) {
		pam_syslog(pamh, LOG_DEBUG, "Failed to open: %s", filename);
		return;
	}

	char buf[MAX_BUF_SIZE] = {0};
	while (!feof(fr)) {
		memset(buf, 0, MAX_BUF_SIZE);
		fread(buf, sizeof(char), MAX_BUF_SIZE, fr);
		pam_syslog(pamh, LOG_DEBUG, "Line: %s", buf);
	}
	fclose(fr);
}

/* pam management functions */

int
pam_sm_authenticate(pam_handle_t *pamh, int flags UNUSED,
                    int argc, const char **argv)
{
    int debug;

    debug = parse_argument(pamh, argc, argv);
    return check_whitelist(pamh, debug);
}

int
pam_sm_setcred (pam_handle_t *pamh UNUSED, int flags UNUSED,
                int argc UNUSED, const char **argv UNUSED)
{
    return PAM_SUCCESS;
}

int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags UNUSED,
                 int argc, const char **argv)
{
    int debug;

    debug = parse_argument(pamh, argc, argv);
    return check_whitelist(pamh, debug);
}

int
pam_sm_chauthtok(pam_handle_t *pamh, int flags UNUSED,
                 int argc, const char **argv)
{
    int debug;

    debug = parse_argument(pamh, argc, argv);
    return check_whitelist(pamh, debug);
}

/* end of pam management functions */


static int
parse_argument(const pam_handle_t *pamh, int argc, const char **argv)
{
    int debug = 0;
    for (;argc-- > 0; ++argv) {
        if (!strcmp(*argv, "debug")) {
            debug |= PAM_DEBUG_ARG;
        } else {
            pam_syslog(pamh, LOG_ERR, "unknown option: %s", *argv);
        }
    }
    return debug;
}

static int
check_whitelist(pam_handle_t *pamh, const int debug)
{
    int ret = 0;

    if (debug | PAM_DEBUG_ARG) {
        pam_syslog(pamh, LOG_DEBUG, "[Debug] uid: %d", getuid());
        pam_syslog(pamh, LOG_DEBUG, "[Debug] pid: %d", getpid());
	char buf[MAX_BUF_SIZE] = {0};
	snprintf(buf, MAX_BUF_SIZE, "/proc/%d/cmdline", getpid());
	print_file(pamh, buf);
    }

    ret = has_in_developer_mode();
    if (debug | PAM_DEBUG_ARG) {
        pam_syslog(pamh, LOG_DEBUG, "[Debug] developer mode: %d", ret);
    }
    if (ret) {
        return PAM_IGNORE;
    }

    //return PAM_IGNORE;
    return PAM_AUTH_ERR;
}

// TODO(jouyouyun): parse json file
static int
has_in_developer_mode()
{
    int ret = 0;
    char buf[MAX_BUF_SIZE] = {0};

    FILE *fr = fopen(DEVELOPER_MODE_FILE, "r");
    if (!fr) {
        return ret;
    }

    while (!feof(fr)) {
        memset(buf, 0, MAX_BUF_SIZE);
        size_t n = fread(buf, sizeof(char), MAX_BUF_SIZE, fr);
        if (n == 0) {
            break;
        }

        char *found = strstr(buf, DEVELOPER_MODE_ENABLE_STR);
        if (found) {
            ret = 1;
            break;
        }
    }

    fclose(fr);
    return ret;
}
