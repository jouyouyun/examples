#include "software_guard.h"
#include "blacklist.h"
#include "netlink_monitor.h"

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
    SoftwareGuardPrivate(SoftwareGuard *soft_guard, const string &blacklist_file);
    ~SoftwareGuardPrivate();

private:
    string GetCmdline(int pid);
    string GetPackageByCmdline(string cmdline);
    string QueryPackageByPath(const string &filepath);
    string QueryPackageForDeepinWine(int pid);
    void FilterCmdlinePrefix(vector<string> & items);
    string GetFileContent(const string & filepath);

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
    string package;

    cout<<"++++++++++ Start handle: "<<pid<<endl;
    string cmdline = d->GetCmdline(pid);
    if (cmdline.empty()) {
        return;
    }

    // wine app
    if (cmdline.find("c:\\") != string::npos ||
            cmdline.find("C:\\") != string::npos) {
        package = d->QueryPackageForDeepinWine(pid);    
    } else {
        package = d->GetPackageByCmdline(cmdline);
    }
    if (package.empty()) {
        return;
    }

    if (strcmp(package.data(), "deepin-wine") == 0||
            strcmp(package.data(), "deepin-wine:i386") == 0 ||
            strcmp(package.data(), "deepin-wine32") == 0 ||
            strcmp(package.data(), "deepin-wine32-preloader:i386") == 0) {
        cout<<"---------Deepin Wine: "<<package<<endl;
        package = d->QueryPackageForDeepinWine(pid);
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
        return;
    }
    this->Kill(package);
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
    string cmdline;
    string program;
    string first;
    vector<string> items;
    vector<string> tmpList;
    char filename[MAX_BUF_SIZE] = {0};

    sprintf(filename, "/proc/%d/cmdline", pid);
    cmdline = GetFileContent(filename);
    if (cmdline.empty()) {
        return "";
    }
    cout<<"==============Content: "<<cmdline<<endl;

    // split by char-terminator
    string tmp(cmdline);
    boost::algorithm::split(items, cmdline,
            boost::algorithm::is_any_of(boost::as_array("\0")));
    FilterCmdlinePrefix(items);

    // split by space
    string googleARCKey("--load-and-launch-app=");
    first = string(items.begin()->data());
    bool chromeARC = (first.find(googleARCKey) != string::npos);
    boost::algorithm::split(tmpList, first, boost::algorithm::is_any_of(" "));
    if (!chromeARC || tmpList.size() < 2) {
        first = string(tmpList.begin()->data());
    } else {
        string tmp1 = string((tmpList.begin()+1)->data());
        first = tmp1.substr(googleARCKey.length());
    }
    cout<<"===============First:"<<first<<", size: "<<items.size()<<endl;

    // if not found in $PATH return empty
    program = boost::process::search_path(first).string();
    if (program.empty()) {
        program = first;
    }
    cout << "Cmdline path: " << program << ", from: '" << tmp << "'" << endl;
    return program;
}

string SoftwareGuardPrivate::GetPackageByCmdline(string cmdline)
{
    string package;

    map<string, string>::iterator it = cmdPkgSet.find(cmdline);
    if (it != cmdPkgSet.end()) {
        cout << "Find in set: " << it->first << " - " << it->second << endl;
        package = string(it->second);
        return package;
    }

    package = QueryPackageByPath(cmdline);
    if (!package.empty()) {
        cmdPkgSet[cmdline] = package;
    }
    return package;
}

string SoftwareGuardPrivate::QueryPackageByPath(const string &filepath)
{
    if (filepath.empty()) {
        return "";
    }

    namespace drunner = dmcg::module::runner;
    string package;
    string program;
    vector<string> args;
    drunner::TaskManager manager;
    boost::shared_ptr<drunner::Task> task;

    if (filepath.find("/") == string::npos) {
        // not absolute path
        return "";
    }
    // TODO(jouyouyun): filter invalid command, such as: 'export'
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

        size_t idx = output.find(": ");
        if (idx == string::npos) {
            package = output;
        } else {
            package = output.substr(0, idx);
        }
    cout << "--------Result for query: " << output<<", package: "<<package<< endl;
    });
    task->Run();

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
    boost::algorithm::split(envList, content,
            boost::algorithm::is_any_of(boost::as_array("\0")));

    string value;
    string wineEnv = "WINEPREFIX=";
    vector<string>::iterator it = envList.begin();
    for (; it != envList.end(); it++) {
        size_t idx = (*it).find(wineEnv);
        if (idx == string::npos) {
            continue;
        }
        value = (*it).substr(idx+1);
        break;
    }
    if (value.empty()) {
        return "";
    }

    vector<string> pathList;
    boost::algorithm::split(pathList, value,
            boost::algorithm::is_any_of("/"));
    vector<string>::iterator app = (pathList.end() - 1);

    memset(filepath, 0, 1024);
    sprintf(filepath, "/opt/deepinwine/apps/%s", app->data());
    return QueryPackageByPath(filepath);
}

string SoftwareGuardPrivate::GetFileContent(const string & filename)
{
    fstream fr;
    string content;

    try {
        fr.open(filename, ios::in);
        istreambuf_iterator<char> beg(fr);
        istreambuf_iterator<char> end;
        content = string(beg, end);
    } catch (exception &e) {
        //LOG_ERROR << "open pid file failed: " << e.what();
        cout << "open pid file failed: " << e.what() << endl;
        return "";
    }

    cout<<"^^^^^^^^^ File: "<<filename<<", Content: "<<content<<endl;
    return content;
}

void SoftwareGuardPrivate::FilterCmdlinePrefix(vector<string> & items)
{
    if (items.size() == 1) {
        return;
    }

    vector<string>::iterator it = items.begin();
    vector<string> invalidPrefix;

    invalidPrefix.push_back("sh");
    invalidPrefix.push_back("/bin/sh");
    invalidPrefix.push_back("bash");
    invalidPrefix.push_back("/bin/bash");
    invalidPrefix.push_back("zsh");
    invalidPrefix.push_back("/bin/zsh");

    if (find(invalidPrefix.begin(), invalidPrefix.end(), string(it->data())) == invalidPrefix.end()) {
        return;
    }
    items.erase(it);

    // filter options
    it = items.begin();
    for(; it != items.end(); it++) {
        if (!(*it).empty() && (*it)[0] == '-') {
            items.erase(it);
            it--;
        } else {
            break;
        }
    }
}
} // namespace software
} // namespace module
} // namespace dmcg
