#include "software_guard.h"
#include "blacklist.h"
#include "netlink_monitor.h"
#include "software_history.h"

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <map>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/range/as_array.hpp>
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
    SoftwareGuardPrivate(SoftwareGuard *soft_guard,
                         const string &whitelist_file,
                         const string &blacklist_file);
    ~SoftwareGuardPrivate();

private:
    string GetProgram(int pid);
    string GetPackage(int pid);
    string GetPackageByProgram(const string &program);
    string QueryPackageByPath(const string &filepath);
    string QueryPackageForDeepinWine(int pid);
    void FilterCmdlinePrefix(vector<string> &items);
    string GetFileContent(const string &filename);
    bool IsShell(const string &name);
    bool IsDeepinWine(const string &name);
    bool IgnoreProgram(const string &program);

    SoftwareGuard *guard;
    unique_ptr<SoftwareHistory> soft_hist;
    unique_ptr<Blacklist> black;
    unique_ptr<Blacklist> white;
    unique_ptr<NetlinkMonitor> monitor;
    map<string, string> cmdPkgSet;
    map<int, string> pidPkgSet;
};

SoftwareGuard::SoftwareGuard()
{
    d = unique_ptr<SoftwareGuardPrivate>(new SoftwareGuardPrivate(this));
}

SoftwareGuard::SoftwareGuard(const string &whitelist_file,
                             const string &blacklist_file)
{
    d = unique_ptr<SoftwareGuardPrivate>(new SoftwareGuardPrivate(this,
                                         whitelist_file,
                                         blacklist_file));
}

SoftwareGuard::~SoftwareGuard()
{}

string SoftwareGuard::DumpSoftwareHistory()
{
    return d->soft_hist->Dump();
}

void SoftwareGuard::ReloadBlacklist(const string &filename)
{
    d->black->SetBlacklist(filename, false);
}

void SoftwareGuard::AppendBlacklist(const string &filename)
{
    d->black->SetBlacklist(filename, true);
}

void SoftwareGuard::ReloadWhitelist(const string &filename)
{
    d->white->SetBlacklist(filename, false);
}

void SoftwareGuard::AppendWhitelist(const string &filename)
{
    d->white->SetBlacklist(filename, true);
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
    string package = d->GetPackage(pid);
    if (package.empty()) {
        return;
    }

    cout<<"+++++++++++[PID]: "<<pid<<" -- [Package]: "<<package<<endl;
    if (d->black->IsInList(package)) {
        //LOG_DEBUG << "Kill pid: " << pid;
        cout << "Kill pid: " << pid << endl;
        // kill program
        int rc = kill(pid, SIGTERM);
        if (rc == -1) {
            //LOG_WARN << "kill proccess pid - " << pid << " failed: " << strerror(errno);
            cout << "kill proccess pid - " << pid << " failed: " << strerror(errno) << endl;
            return;
        }

        // emit signal 'Kill(package)'
        this->Kill(package);
        return;
    }

    if (d->white->IsInList(package)) {
        // record pid --> pkg
        d->pidPkgSet[pid] = package;
        // save startup
        d->soft_hist->SaveStartup(package);
    }
}

void SoftwareGuard::HandleExitEvent(int pid)
{
    map<int, string>::iterator it = d->pidPkgSet.find(pid);
    if (it == d->pidPkgSet.end()) {
        return ;
    }
    string package = string(it->second);
    // save shutdown
    d->soft_hist->SaveShutdown(package);
    d->pidPkgSet.erase(it);
}

SoftwareGuardPrivate::SoftwareGuardPrivate(SoftwareGuard *soft_guard): guard(soft_guard)
{
    soft_hist = unique_ptr<SoftwareHistory>(new SoftwareHistory);
    black = unique_ptr<Blacklist>(new Blacklist);
    monitor = unique_ptr<NetlinkMonitor>(new NetlinkMonitor(guard));
    cmdPkgSet.clear();
    pidPkgSet.clear();
}

SoftwareGuardPrivate::SoftwareGuardPrivate(SoftwareGuard *soft_guard,
        const std::string &whitelist_file,
        const string &blacklist_file): guard(soft_guard)
{
    soft_hist = unique_ptr<SoftwareHistory>(new SoftwareHistory);
    black = unique_ptr<Blacklist>(new Blacklist(blacklist_file));
    white = unique_ptr<Blacklist>(new Blacklist(whitelist_file));
    monitor = unique_ptr<NetlinkMonitor>(new NetlinkMonitor(guard));
    cmdPkgSet.clear();
    pidPkgSet.clear();
}

SoftwareGuardPrivate::~SoftwareGuardPrivate()
{
    cmdPkgSet.clear();
    pidPkgSet.clear();
}

string SoftwareGuardPrivate::GetPackage(int pid)
{
    string package;
    string program = GetProgram(pid);
    if (program.empty()) {
        return "";
    }

    // wine app
    if (program.find("c:\\") != string::npos ||
            program.find("C:\\") != string::npos) {
        package = QueryPackageForDeepinWine(pid);
    } else {
        package = GetPackageByProgram(program);
    }
    if (package.empty()) {
        return package;
    }

    if (IsDeepinWine(package)) {
        package = QueryPackageForDeepinWine(pid);
    }
    return package;
}

string SoftwareGuardPrivate::GetProgram(int pid)
{
    fstream fr;
    char filename[MAX_BUF_SIZE];
    string cmdline;
    string program;
    string first;
    string arcAppKey("--load-and-launch-app=");
    bool isARCApp;
    vector<string> items;
    vector<string> tmpList;

    memset(filename, 0, MAX_BUF_SIZE);
    sprintf(filename, "/proc/%d/cmdline", pid);
    cmdline = GetFileContent(filename);
    if (cmdline.empty()) {
        return "";
    }

    // split by char-terminator
    namespace ba = boost::algorithm;
    ba::split(items, cmdline,
              ba::is_any_of(boost::as_array("\0")));
    FilterCmdlinePrefix(items);

    // split by space
    first = string(items.begin()->data());
    isARCApp = (first.find(arcAppKey) != string::npos);
    ba::split(tmpList, first, ba::is_any_of(" "));
    // google chrome arc app
    if (!isARCApp || tmpList.size() < 2) {
        first = string(tmpList.begin()->data());
    } else {
        first = string((tmpList.begin() + 1)->data()).substr(arcAppKey.length());
    }

    program = boost::process::search_path(first).string();
    if (program.empty()) {
        program = first;
    }

    return program;
}

string SoftwareGuardPrivate::GetPackageByProgram(const string &program)
{
    string package;

    map<string, string>::iterator it = cmdPkgSet.find(program);
    if (it != cmdPkgSet.end()) {
        package = string(it->second);
        return package;
    }

    package = QueryPackageByPath(program);
    if (!package.empty()) {
        cmdPkgSet[program] = package;
    }
    return package;
}

string SoftwareGuardPrivate::QueryPackageForDeepinWine(int pid)
{
    string content;
    vector<string> envList;
    char filepath[MAX_BUF_SIZE] = {0};

    sprintf(filepath, "/proc/%d/environ", pid);
    content = GetFileContent(filepath);
    if (content.empty()) {
        return "";
    }

    namespace ba = boost::algorithm;
    ba::split(envList, content,
              ba::is_any_of(boost::as_array("\0")));
    string value;
    string wineEnv = "WINEPREFIX=";
    vector<string>::iterator it = envList.begin();

    for (; it != envList.end(); it++) {
        size_t idx = (*it).find(wineEnv);
        if (idx == string::npos) {
            continue;
        }
        value = (*it).substr(idx + 1);
        break;
    }
    if (value.empty()) {
        return "";
    }

    vector<string> list;
    ba::split(list, value, ba::is_any_of("/"));
    vector<string>::iterator app = (list.end() - 1);
    memset(filepath, 0, MAX_BUF_SIZE);
    sprintf(filepath, "/opt/deepinwine/apps/%s", app->data());
    return QueryPackageByPath(filepath);
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
    cout<<"-----------File for query: "<<filepath<<endl;
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

        size_t idx = output.find(": ");
        if (idx == string::npos) {
            package = output;
        } else {
            package = output.substr(0, idx);
        }
    });
    task->Run();

    return package;
}

void SoftwareGuardPrivate::FilterCmdlinePrefix(vector<string> &items)
{
    if (items.size() == 1) {
        return ;
    }

    vector<string>::iterator it = items.begin();

    if (!IsShell(string(it->data()))) {
        return;
    }
    items.erase(it);

    // filter options
    it = items.begin();
    for (; it != items.end(); it++) {
        if (!(*it).empty() && (*it)[0] == '-') {
            items.erase(it);
            it--;
        } else {
            break;
        }
    }
}

string SoftwareGuardPrivate::GetFileContent(const string &filename)
{
    fstream fr;
    string content;

    try {
        fr.open(filename, ios::in);
        istreambuf_iterator<char> beg(fr);
        istreambuf_iterator<char> end;
        content = string(beg, end);
    } catch (exception &e) {
        //LOG_ERROR << "read file content failed: " << e.what();
        cout << "read file content failed: " << e.what() << endl;
        return "";
    }

    return content;
}

bool SoftwareGuardPrivate::IsShell(const string &name)
{
    static vector<string> list;

    list.push_back("sh");
    list.push_back("bash");
    list.push_back("zsh");
    list.push_back("/bin/sh");
    list.push_back("/bin/bash");
    list.push_back("/bin/zsh");

    return find(list.begin(), list.end(), name) != list.end();
}

bool SoftwareGuardPrivate::IsDeepinWine(const string &name)
{
    static vector<string> list;

    list.push_back("deepin-wine");
    list.push_back("deepin-wine32");
    list.push_back("deepin-wine:i386");
    list.push_back("deepin-wine32-preloader:i386");

    return find(list.begin(), list.end(), name) != list.end();
}

bool SoftwareGuardPrivate::IgnoreProgram(const string &program)
{
    static vector<string> list = {"/usr/bin/whoami", "whoami", "export", "gvfs-backends", "dbus-send"};
    //return find(list.begin(), list.end(), string(program)) == list.end();
    vector<string>::iterator iter = list.begin();
    for (; iter != list.end(); iter++) {
    cout<<"[[[[[[[[Ignore]]]]]]]]: '"<<program.c_str()<<"', SRC: '"<<*iter->data()<<"',"<<endl;
        if (strcmp(iter->data(), program.c_str()) == 0) {
            return false;
        }
    }
    return true;
}
} // namespace software
} // namespace module
} // namespace dmcg
