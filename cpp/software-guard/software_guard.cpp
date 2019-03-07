#include "software_guard.h"
#include "blacklist.h"
#include "netlink_monitor.h"

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include <iostream>
// #include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

#define MAX_BUF_SIZE 1024

class SoftwareGuardPrivate
{
    friend class SoftwareGuard;

public:
    SoftwareGuardPrivate(SoftwareGuard *soft_guard);
    SoftwareGuardPrivate(SoftwareGuard *soft_guard, const string &blacklist_file);
    ~SoftwareGuardPrivate();

private:
    string GetCmdline(int pid);

    SoftwareGuard *guard;
    unique_ptr<Blacklist> black;
    unique_ptr<NetlinkMonitor> monitor;
};

SoftwareGuard::SoftwareGuard()
{
    d = unique_ptr<SoftwareGuardPrivate>(new SoftwareGuardPrivate(this));
}

SoftwareGuard::SoftwareGuard(const string &blacklist_file)
{
    d = unique_ptr<SoftwareGuardPrivate>(new SoftwareGuardPrivate(this, blacklist_file));
}

SoftwareGuard::~SoftwareGuard()
{}

void SoftwareGuard::ReloadBlacklist(const string &filename)
{
    d->black->SetBlacklist(filename, false);
}

void SoftwareGuard::AppendBlacklist(const string &filename)
{
    d->black->SetBlacklist(filename, true);
}

void SoftwareGuard::HandleExecEvent(int pid)
{
    string cmdline = d->GetCmdline(pid);
    if (cmdline.empty()) {
        return;
    }

    cout << "Cmdline: " << cmdline << endl;
    if (!d->black->IsInList(cmdline)) {
        return;
    }

    cout << "Kill pid: " << pid << endl;
    // kill program
    int rc = kill(pid, SIGTERM);
    if (rc == -1) {
        // LOG_WARN << "kill proccess pid - " << pid << " failed: " << strerror(errno);
        cout << "Kill proccess id - " << pid << " failed: " << strerror(errno) << endl;
    }
}

SoftwareGuardPrivate::SoftwareGuardPrivate(SoftwareGuard *soft_guard): guard(soft_guard)
{
    black = unique_ptr<Blacklist>(new Blacklist);
    monitor = unique_ptr<NetlinkMonitor>(new NetlinkMonitor(guard));
}

SoftwareGuardPrivate::SoftwareGuardPrivate(SoftwareGuard *soft_guard,
        const string &blacklist_file): guard(soft_guard)
{
    black = unique_ptr<Blacklist>(new Blacklist(blacklist_file));
    monitor = unique_ptr<NetlinkMonitor>(new NetlinkMonitor(guard));
}

void SoftwareGuard::Loop()
{
    d->monitor->Loop();
}

void SoftwareGuard::Quit()
{
    d->monitor->Quit();
}

SoftwareGuardPrivate::~SoftwareGuardPrivate()
{}

string SoftwareGuardPrivate::GetCmdline(int pid)
{
    fstream fr;
    char filename[MAX_BUF_SIZE];
    string cmdline;

    memset(filename, 0, MAX_BUF_SIZE);
    sprintf(filename, "/proc/%d/cmdline", pid);

    try {
        fr.open(filename, ios::in);
        istreambuf_iterator<char> beg(fr);
        istreambuf_iterator<char> end;
        cmdline = string(beg, end);
        boost::algorithm::trim_right_if(cmdline,
                                        boost::algorithm::is_any_of("\n"));
    } catch (exception &e) {
        // LOG_ERROR << "open pid file failed: " << e.what();
        cout << "open pid file failed: " << e.what() << endl;
        return "";
    }
    return cmdline;
}
} // namespace software
} // namespace module
} // namespace dmcg
