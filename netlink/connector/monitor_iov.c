/**
 * compile: gcc -Wall -g monitor_iov.c -o monitor_iov
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

#define MAX_PAYLOAD 1024

static int nl_connect();
static int set_event_listen(int nl_sock, int enabled);
static int handle_event(int nl_sock);
static char* get_pid_cmdline(int pid);

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
    struct msghdr msg;
    // must be pointer
    struct nlmsghdr *nlhdr;
    struct sockaddr_nl nladdr;
    struct cn_msg *cnmsg;
    struct iovec iov;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = PF_NETLINK;
    nladdr.nl_pid = 0;
    nladdr.nl_groups = 0;

    nlhdr = (struct nlmsghdr*)malloc(sizeof(struct nlmsghdr));
    memset(nlhdr, 0, sizeof(struct nlmsghdr));
    nlhdr->nlmsg_pid = getpid();
    nlhdr->nlmsg_type = NLMSG_DONE;
    nlhdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct nlmsghdr) +
                                    sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op));
    cnmsg = (struct cn_msg*)NLMSG_DATA(nlhdr);
    cnmsg->id.idx = CN_IDX_PROC;
    cnmsg->id.val = CN_VAL_PROC;
    cnmsg->len = sizeof(enum proc_cn_mcast_op);
    *((int*)(cnmsg->data)) = enabled ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void*)&nladdr;
    msg.msg_namelen = sizeof(nladdr);

    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void*)nlhdr;
    iov.iov_len = nlhdr->nlmsg_len;

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    rc = sendmsg(nl_sock, &msg, 0);
    free(nlhdr);
    nlhdr = NULL;
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
    struct msghdr msg;
    struct sockaddr_nl nladdr;
    struct nlmsghdr nlhdr;
    struct cn_msg *cnmsg;
    struct proc_event *ev;
    struct iovec iov;

    while (1) {
        memset(&msg, 0, sizeof(msg));
        memset(&nladdr, 0, sizeof(nladdr));
        memset(&nlhdr, 0, sizeof(struct nlmsghdr));
        msg.msg_name = (void*)&nladdr;
        msg.msg_namelen = sizeof(nladdr);
        memset(&iov, 0, sizeof(iov));
        nlhdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct nlmsghdr) +
                                        sizeof(struct cn_msg) +
                                        sizeof(struct proc_event));
        iov.iov_base = (void*)&nlhdr;
        iov.iov_len = MAX_PAYLOAD;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        rc = recvmsg(nl_sock, &msg, 0);
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

        if (nladdr.nl_family != PF_NETLINK) {
            continue;
        }

        /* fprintf(stderr, "[NLHDR] pid: %d, type: %d, len: %d\n", */
                /* nlhdr->nlmsg_pid, nlhdr->nlmsg_type, nlhdr->nlmsg_len); */
        cnmsg = (struct cn_msg*)NLMSG_DATA(&nlhdr);
        /* fprintf(stderr, "[CNMSG] proc: %d, len: %d, ev len: %lu\n", cnmsg->id.idx == CN_IDX_PROC, */
                /* cnmsg->len, sizeof(struct proc_event)); */
        if (cnmsg->len != sizeof(struct proc_event)) {
            fprintf(stderr, "invalid event\n");
            continue;
        }

        ev = (struct proc_event*)cnmsg->data;
        switch (ev->what) {
        case PROC_EVENT_NONE:
            fprintf(stdout, "Set mcast listen ok!\n");
            break;
        case PROC_EVENT_EXEC: {
            fprintf(stdout, "[Event] type: EXEC\n");
            pid = ev->event_data.exec.process_pid;
            char *cmdline = get_pid_cmdline(pid);
            if (!cmdline) {
                break;
            }
            fprintf(stdout, "Process launch, pid: %d, cmdline: %s\n",
                    pid, cmdline);
            free(cmdline);
            cmdline = NULL;
            break;
        }
        case PROC_EVENT_EXIT:
            fprintf(stdout, "[Event] type: EIXT\n");
            pid = ev->event_data.exec.process_pid;
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
    char* cmdline;

    /* fprintf(stdout, "[DEBUG] get pid cmdline: %d\n", pid); */
    if (pid < 1) {
        return NULL;
    }

    memset(filename, 0, 1024);
    sprintf(filename, "/proc/%d/cmdline", pid);
    fr = fopen(filename, "r");
    if (!fr) {
        fprintf(stderr, "Failed to open pid file(%s): %s\n",
                filename, strerror(errno));
        return NULL;
    }

    memset(buf, 0, 1024);
    if (!fgets(buf, 1024, fr)) {
        fclose(fr);
        if (errno == 0) {
            return NULL;
        }
        fprintf(stderr, "Failed to read pid file(%s): %s\n",
                filename, strerror(errno));
        return NULL;
    }
    fclose(fr);

    // Why calloc will SIGINT when free?
    // Error: 'free(): invalid next size (fast) '
    cmdline = (char*)malloc((strlen(buf) + 1)*sizeof(char));
    if (!cmdline) {
        return NULL;
    }
    memset(cmdline, 0, (strlen(buf) + 1) * sizeof(char));
    memcpy(cmdline, buf, strlen(buf));
    return cmdline;
}
