/**
 * compile: gcc -Wall -g monitor.c -o monitor
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

static int nl_connect();
static int set_event_listen(int nl_sock, int enabled);
static int handle_event(int nl_sock);
static char* get_pid_cmdline(int pid);
void fix_cmdline(char *buf, size_t num);

int
main(int argc, char* argv[])
{
    int rc = 0;
    int nl_sock;

    nl_sock = nl_connect();
    if (nl_sock == -1) {
        return -1;
    }

    rc = set_event_listen(nl_sock, 1);
    if (rc == -1) {
        rc = EXIT_FAILURE;
        goto out;
    }

    rc = handle_event(nl_sock);
    if (rc == -1) {
        rc = EXIT_FAILURE;
    }

    set_event_listen(nl_sock, 0);

out:
    close(nl_sock);
    return rc;
}

static int
nl_connect()
{
    int rc;
    int nl_sock;
    struct sockaddr_nl sa_nl;

    nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nl_sock == -1) {
        fprintf(stderr, "Failed to open socket: %s\n", strerror(errno));
        return -1;
    }

    memset(&sa_nl, 0, sizeof(sa_nl));
    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    rc = bind(nl_sock, (struct sockaddr*)&sa_nl, sizeof(sa_nl));
    if (rc == -1) {
        close(nl_sock);
        fprintf(stderr, "Failed to bind: %s\n", strerror(errno));
        return -1;
    }

    return nl_sock;
}

static int set_event_listen(int nl_sock, int enabled)
{
    int rc;
    struct __attribute__((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__((__packed__)){
            struct cn_msg cn_msg;
            enum proc_cn_mcast_op cn_mcast;
        };
    } nlcn_msg;

    memset(&nlcn_msg, 0, sizeof(nlcn_msg));
    nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
    nlcn_msg.nl_hdr.nlmsg_pid = getpid();
    nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;
    nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
    nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
    nlcn_msg.cn_msg.len  = sizeof(enum proc_cn_mcast_op);
    nlcn_msg.cn_mcast = enabled ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
    if (rc == -1) {
        fprintf(stderr, "Failed to enable listen: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static int handle_event(int nl_sock)
{
    int rc;
    int pid;
    char *cmdline;
    struct __attribute__((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__((__packed__)){
            struct cn_msg cn_msg;
            struct proc_event ev;
        };
    } nlcn_msg;

    while (1) {
        memset(&nlcn_msg, 0, sizeof(nlcn_msg));
        rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
        if (rc == 0) {
            // shutdown
            return 0;
        } else if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "Failed to recieve: %s\n", strerror(errno));
            return -1;
        }

        switch (nlcn_msg.ev.what) {
        case PROC_EVENT_NONE:
            fprintf(stdout, "Set mcast listen ok!\n");
            break;
        /*case PROC_EVENT_FORK: {*/
            /*int parent, child;*/
            /*parent = nlcn_msg.ev.event_data.fork.parent_pid;*/
            /*cmdline = get_pid_cmdline(parent);*/
            /*if (!cmdline) {*/
                /*break;*/
            /*}*/
            /*fprintf(stdout, "Fork parent: %d, cmdline: %s\n",*/
                    /*parent, cmdline);*/
            /*free(cmdline);*/

            /*child = nlcn_msg.ev.event_data.fork.child_pid;*/
            /*cmdline = get_pid_cmdline(child);*/
            /*if (!cmdline) {*/
                /*break;*/
            /*}*/
            /*fprintf(stdout, "Fork child: %d, cmdline: %s\n",*/
                    /*child, cmdline);*/
            /*free(cmdline);*/
        /*}*/
        case PROC_EVENT_EXEC:
            pid = nlcn_msg.ev.event_data.exec.process_pid;
            cmdline = get_pid_cmdline(pid);
            if (!cmdline) {
                break;
            }
            fprintf(stdout, "Process launch, pid: %d, cmdline: %s\n",
                    pid, cmdline);
            free(cmdline);
            break;
        case PROC_EVENT_EXIT:
            pid = nlcn_msg.ev.event_data.exec.process_pid;
            fprintf(stdout, "Process exit: %d\n", pid);
            break;
        default:
            break;
        }
    }

    return 0;
}

static char*
get_pid_cmdline(int pid)
{
    FILE* fr;
    static char filename[1024];
    static char buf[1024];
    char* content;

    memset(filename, 0, 1024);
    sprintf(filename, "/proc/%d/cmdline", pid);
    fr = fopen(filename, "r");
    if (!fr) {
        fprintf(stderr, "Failed to open pid file(%s): %s\n",
                filename, strerror(errno));
        return NULL;
    }

    while(!feof(fr)) {
    memset(buf, 0, 1024);
        size_t num = fread(buf, 1, 1024, fr);
        if (num != strlen(buf)) {
            fix_cmdline(buf, num);
        }
        fprintf(stdout, "Read %lu chars: %s\n", num, buf);
    }
/*    if (!fgets(buf, 1024, fr)) {*/
        /*fclose(fr);*/
        /*if (errno == 0) {*/
            /*return NULL;*/
        /*}*/
        /*fprintf(stderr, "Failed to read pid file(%s): %s\n",*/
                /*filename, strerror(errno));*/
        /*return NULL;*/
    /*}*/
    fclose(fr);

    content = (char*)calloc(strlen(buf) + 1, 1);
    memcpy(content, buf, strlen(buf));
    return content;
}

void fix_cmdline(char *buf, size_t num)
{
    size_t i = 0;
    for (; i < num-1; i++) {
        if (buf[i] == '\0') {
            buf[i] = ' ';
        }
    }
}
