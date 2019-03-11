#include "software_guard.h"
#include "blacklist.h"
#include "netlink_monitor.h"

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <map>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>

#include "../runner/task_manager.h"
#include "../runner/task.h"
#include <iostream>
//#include "modules/log/log.h"

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
    string GetPackageByCmdline(string cmdline);
    string QueryPackageByPath(const string &filepath);

    SoftwareGuard *guard;
    unique_ptr<Blacklist> black;
    unique_ptr<NetlinkMonitor> monitor;
    map<string, string> cmdPkgSet;
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

void SoftwareGuard::Loop()
{
    d->monitor->Loop();
}

void SoftwareGuard::Quit()
{
    d->monitor->Quit();
}

void SoftwareGuard::HandleExecEvent(int pid)
{
    string cmdline = d->GetCmdline(pid);
    if (cmdline.empty()) {
        return;
    }

    string package = d->GetPackageByCmdline(cmdline);
    if (package.empty()) {
        return;
    }

    if (!d->black->IsInList(package)) {
        return;
    }

    //LOG_DEBUG << "Kill pid: " << pid << endl;
    cout << "Kill pid: " << pid << endl;
    // kill program
    int rc = kill(pid, SIGTERM);
    if (rc == -1) {
        //LOG_WARN << "kill proccess pid - " << pid << " failed: " << strerror(errno);
        cout << "kill proccess pid - " << pid << " failed: " << strerror(errno) << endl;
    }
}

SoftwareGuardPrivate::SoftwareGuardPrivate(SoftwareGuard *soft_guard): guard(soft_guard)
{
    black = unique_ptr<Blacklist>(new Blacklist);
    monitor = unique_ptr<NetlinkMonitor>(new NetlinkMonitor(guard));
    cmdPkgSet.clear();
}

SoftwareGuardPrivate::SoftwareGuardPrivate(SoftwareGuard *soft_guard,
        const string &blacklist_file): guard(soft_guard)
{
    black = unique_ptr<Blacklist>(new Blacklist(blacklist_file));
    monitor = unique_ptr<NetlinkMonitor>(new NetlinkMonitor(guard));
    cmdPkgSet.clear();
}

SoftwareGuardPrivate::~SoftwareGuardPrivate()
{
    cmdPkgSet.clear();
}

string SoftwareGuardPrivate::GetCmdline(int pid)
{
    fstream fr;
    char filename[MAX_BUF_SIZE];
    string cmdline;

    memset(filename, 0, MAX_BUF_SIZE);
    sprintf(filename, "/proc/%d/cmdline", pid);

    cout << ">>>>>>>>>>Cmdline pid: " << pid << endl;
    try {
        fr.open(filename, ios::in);
        istreambuf_iterator<char> beg(fr);
        istreambuf_iterator<char> end;
        cmdline = string(beg, end);
        boost::algorithm::trim_right_if(cmdline,
                                        boost::algorithm::is_any_of("\n"));
    } catch (exception &e) {
        //LOG_ERROR << "open pid file failed: " << e.what();
        cout << "open pid file failed: " << e.what() << endl;
        return "";
    }
    return cmdline;
}

string SoftwareGuardPrivate::GetPackageByCmdline(string cmdline)
{
    string program;
    string package;
    string first;
    vector<string> items;

    string tmp(cmdline);
    boost::algorithm::split(items, cmdline, boost::algorithm::is_any_of(" "));
    first = string(items.begin()->data());
    // if not found in $PATH return empty
    program = boost::process::search_path(first).string();
    if (program.empty()) {
        program = first;
    }
    cout << "Cmdline path: " << program << ", from: '" << tmp << "'" << endl;
    map<string, string>::iterator it = cmdPkgSet.find(program);
    if (it != cmdPkgSet.end()) {
        cout << "Find in set: " << it->first << " - " << it->second << endl;
        package = string(it->second);
        return package;
    }

    package = QueryPackageByPath(program);
    if (!package.empty()) {
        cmdPkgSet[program] = package;
    }
    return package;
}

string SoftwareGuardPrivate::QueryPackageByPath(const string &filepath)
{
    namespace drunner = dmcg::module::runner;
    string package;
    string program;
    vector<string> args;
    drunner::TaskManager manager;
    boost::shared_ptr<drunner::Task> task;

    program = "dpkg";
    args.push_back("-S");
    args.push_back(filepath);
    task = manager.Create(program, args);
    task->Finish.connect([&package](const string & exception,
                                    int exit_code,
                                    const string & output,
    const string & error_output) {
        if (!exception.empty()) {
            //LOG_WARN << "query package by path failed: " << exception;
            cout << "query package by path failed: " << exception << endl;
            return;
        }

        cout << "--------Result for query: " << output << endl;
        vector<string> items;
        boost::algorithm::split(items, output,
                                boost::algorithm::is_any_of(":"));
        package = (items.begin()->data());
    });
    task->Run();

    return package;
}
} // namespace software
} // namespace module
} // namespace dmcg
