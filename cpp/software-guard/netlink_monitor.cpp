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

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>

#include <iostream>
//#include "modules/log/log.h"

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
    void RealHandleEvent(bool exec, int pid);

private:
    int Connect();
    int SubscribeEvent(bool enabled);
    int HandleEvent();

    bool need_exit;
    int nlsock;
    NetlinkProc *handler;
    boost::asio::io_service ioService;
    // 声明一个 work 的原因是为了保证ioService的run方法在这个work销毁
    // 之前不会退出
    unique_ptr<boost::asio::io_service::work> work;
    boost::thread_group thrdGrp;
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

    work = unique_ptr<boost::asio::io_service::work>(new  boost::asio::io_service::work(this->ioService));
    int n = boost::thread::hardware_concurrency();
    for (int i = 0; i < n * 2; i++) {
        this->thrdGrp.create_thread(boost::bind(&boost::asio::io_service::run,
                                                &this->ioService));
    }
}

NLMonitorPrivate::~NLMonitorPrivate()
{
    if (nlsock > 0) {
        close(nlsock);
        nlsock = -1;
    }

    ioService.stop();
    thrdGrp.join_all();
}

int NLMonitorPrivate::Connect()
{
    int rc;
    struct sockaddr_nl nladdr;

    nlsock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nlsock == -1) {
        //LOG_ERROR << "open netlink socket failed: " << strerror(errno);
        cout << "open netlink socket failed: " << strerror(errno) << endl;
        return -1;
    }

    // ENOBUF
    // There are several reasons why you may hit ENOBUFS:
    // 1. your program is too slow to handle the Netlink messages that you
    //    receive from the kernel at a given rate. This is easier to trigger if
    //    the handling that you perform on every message takes too long.
    // 2. the queue size is too small
    // Hit by setsockopt
    // 1. setsockopt(nfct_fd(h), SOL_NETLINK, NETLINK_BROADCAST_SEND_ERROR, &on, sizeof(int));
    // 2. setsockopt(nfct_fd(h), SOL_NETLINK, NETLINK_NO_ENOBUFS, &on, sizeof(int));
    // See: https://netfilter.vger.kernel.narkive.com/cwzlgk8d/why-no-buffer-space-available
    int on = 1;
    setsockopt(nlsock, SOL_NETLINK, NETLINK_BROADCAST_ERROR,
               &on, sizeof(int));
    setsockopt(nlsock, SOL_NETLINK, NETLINK_NO_ENOBUFS,
               &on, sizeof(int));

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_groups = CN_IDX_PROC;
    nladdr.nl_pid = getpid();

    rc = bind(nlsock, (struct sockaddr *)&nladdr, sizeof(nladdr));
    if (rc == -1) {
        close(nlsock);
        nlsock = -1;
        //LOG_ERROR << "bind netlink socket failed: " << strerror(errno);
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
        //LOG_ERROR << "subscribe netlink event failed: " << strerror(errno);
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
    size_t len = sizeof(msg);

    while (!need_exit) {
        memset(&msg, 0, len);
        rc = 0;
        rc = recv(nlsock, &msg, len, MSG_WAITALL);
        if (rc == 0) {
            // shutdown
            need_exit = true;
            break;
        } else if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            need_exit = true;
            //LOG_WARN << "recieve netlink socket failed: " << strerror(errno);
            cout << "recieve netlink socket failed: " << strerror(errno) << endl;
            break;
        }

        switch (msg.event.what) {
        case proc_event::PROC_EVENT_NONE:
            //LOG_INFO << "Subscribe mcast event success!";
            cout << "Subscribe mcast event success!" << endl;
            break;
        case proc_event::PROC_EVENT_EXEC: {
            int pid = msg.event.event_data.exec.process_pid;
            // cout<<"~~~~~~~ [Exec] Start pid: "<<pid<<" ~~~~~~"<<endl;
            //handler->HandleExecEvent(pid);
            this->ioService.post(boost::bind(&NLMonitorPrivate::RealHandleEvent,
                                             this, true, pid));
            break;
        }
        case proc_event::PROC_EVENT_EXIT: {
            int pid = msg.event.event_data.exit.process_pid;
            // cout<<"======= [Exit] Start pid: "<<pid<<" ======="<<endl;
            //handler->HandleExitEvent(pid);
            this->ioService.post(boost::bind(&NLMonitorPrivate::RealHandleEvent,
                                             this, false, pid));
            break;
        }
        default:
            break;
        }
    }

    return rc;
}

void NLMonitorPrivate::RealHandleEvent(bool exec, int pid)
{
    if (exec) {
        this->handler->HandleExecEvent(pid);
        return;
    }
    this->handler->HandleExitEvent(pid);
}
} // namespace software
} // namespace module
} // namespace dmcg
