#include "netlink_monitor.h"
#include "netlink_proc.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

#include <iostream>
// #include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

class NLMonitorPrivate
{
    friend class NetlinkMonitor;

public:
    NLMonitorPrivate(NetlinkProc *proc);
    ~NLMonitorPrivate();

private:
    int Connect();
    int SubscribeEvent(bool enabled);
    int HandleEvent();

    bool need_exit;
    int nlsock;
    NetlinkProc *handler;
};

NetlinkMonitor::NetlinkMonitor(NetlinkProc *proc)
{
    d = unique_ptr<NLMonitorPrivate>(new NLMonitorPrivate(proc));
}

NetlinkMonitor::~NetlinkMonitor()
{}

int NetlinkMonitor::Loop()
{
    int rc;

    rc = d->Connect();
    if (rc == -1) {
        return -1;
    }

    rc = d->SubscribeEvent(true);
    if (rc == -1) {
        return -1;
    }

    // block
    rc = d->HandleEvent();
    if (rc == -1) {
        return -1;
    }

    d->SubscribeEvent(false);
    return 0;
}

void NetlinkMonitor::Quit()
{
    if (d->need_exit) {
        return;
    }

    d->need_exit = true;
    d->SubscribeEvent(false);
}

NLMonitorPrivate::NLMonitorPrivate(NetlinkProc *proc): handler(proc)
{
    need_exit = false;
    nlsock = -1;
}

NLMonitorPrivate::~NLMonitorPrivate()
{
    if (nlsock > 0) {
        close(nlsock);
        nlsock = -1;
    }
}

int NLMonitorPrivate::Connect()
{
    int rc;
    struct sockaddr_nl nladdr;

    nlsock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nlsock == -1) {
        // LOG_ERROR << "open netlink socket failed: " << strerror(errno);
        cout << "open netlink socket failed: " << strerror(errno) << endl;
        return -1;
    }

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_groups = CN_IDX_PROC;
    nladdr.nl_pid = getpid();

    rc = bind(nlsock, (struct sockaddr *)&nladdr, sizeof(nladdr));
    if (rc == -1) {
        close(nlsock);
        nlsock = -1;
        // LOG_ERROR << "bind netlink socket failed: " << strerror(errno);
        cout << "bind netlink socket failed: " << strerror(errno) << endl;
        return -1;
    }

    return 0;
}

int NLMonitorPrivate::SubscribeEvent(bool enabled)
{
    struct __attribute__((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr hdr;
        struct __attribute__((__packed__)) {
            struct cn_msg cnmsg;
            enum proc_cn_mcast_op mcast;
        };
    } msg;
    int rc;

    memset(&msg, 0, sizeof(msg));
    msg.hdr.nlmsg_len = sizeof(msg);
    msg.hdr.nlmsg_pid = getpid();
    msg.hdr.nlmsg_type = NLMSG_DONE;
    msg.cnmsg.id.idx = CN_IDX_PROC;
    msg.cnmsg.id.val = CN_VAL_PROC;
    msg.cnmsg.len = sizeof(enum proc_cn_mcast_op);
    msg.mcast = enabled ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    rc = send(nlsock, &msg, sizeof(msg), 0);
    if (rc == -1) {
        // LOG_ERROR << "subscribe netlink event failed: " << strerror(errno);
        cout << "subscribe netlink event failed: " << strerror(errno) << endl;
        return -1;
    }

    return 0;
}

int NLMonitorPrivate::HandleEvent()
{
    struct __attribute__((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr hdr;
        struct __attribute__((__packed__)) {
            struct cn_msg cnmsg;
            struct proc_event event;
        };
    } msg;
    int rc;

    while (!need_exit) {
        memset(&msg, 0, sizeof(msg));
        rc = 0;
        rc = recv(nlsock, &msg, sizeof(msg), 0);
        if (rc == 0) {
            // shutdown
            need_exit = true;
            break;
        } else if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            need_exit = true;
            // LOG_WARN << "recieve netlink socket failed: " << strerror(errno);
            cout << "recieve netlink socket failed: " << strerror(errno) << endl;
            break;
        }

        switch (msg.event.what) {
        case proc_event::PROC_EVENT_NONE:
            // LOG_INFO << "Subscribe mcast event success!";
            cout << "Subscribe mcast event success!" << endl;
            break;
        case proc_event::PROC_EVENT_EXEC: {
            int pid = msg.event.event_data.exec.process_pid;
            handler->HandleExecEvent(pid);
            break;
        }
        default:
            break;
        }
    }

    return rc;
}
} // namespace software
} // namespace module
} // namespace dmcg
