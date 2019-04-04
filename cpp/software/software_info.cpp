#include "software_info.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include "task_manager.h"
#include "task.h"
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
#define PROGRAM_DPKG_QUERY "dpkg-query"
#define CMDLINE "dpkg-query -f '${Package},${Version},${Architecture}\n' -W"
#define CMDLINE_DELIM ","

void ParseCMDLine(string &line, SoftwareInfo *info);

class SoftwareInfoListPrivate
{
    friend class SoftwareInfoList;
public:
    SoftwareInfoListPrivate(const string &package);
    ~SoftwareInfoListPrivate();
private:
    vector<string> lines;
    vector<string>::iterator it;

    vector<string> MakeDpkgQueryArgs(const string &name);
};

SoftwareInfo::SoftwareInfo()
{
}

SoftwareInfo::~SoftwareInfo()
{
}

SoftwareInfo::SoftwareInfo(const string &name)
{
    SoftwareInfoList soft(name);

    SoftwareInfo *info = soft.Next();
    if (!info) {
        return;
    }
    this->name = info->name;
    this->version = info->version;
    this->architecture = info->architecture;
}

SoftwareInfoList::SoftwareInfoList()
{
    d = unique_ptr<SoftwareInfoListPrivate>(new SoftwareInfoListPrivate(""));
}

SoftwareInfoList::SoftwareInfoList(const string &name)
{
    d = unique_ptr<SoftwareInfoListPrivate>(new SoftwareInfoListPrivate(name));
}

SoftwareInfoList::~SoftwareInfoList()
{}

SoftwareInfo *SoftwareInfoList::Next()
{
    if (d->lines.size() == 0 || d->it == d->lines.end()) {
        return NULL;
    }

    string line = string(d->it->data());
    SoftwareInfo *info = new SoftwareInfo;
    ParseCMDLine(line, info);
    (d->it)++;
    return info;
}

SoftwareInfoListPrivate::SoftwareInfoListPrivate(const string &name)
{
    namespace drunner = dmcg::module::runner;
    drunner::TaskManager manager;
    boost::shared_ptr<drunner::Task> task;
    vector<string> args;

    args = MakeDpkgQueryArgs(name);
    task = manager.Create(PROGRAM_DPKG_QUERY, args);
    task->Finish.connect([this](
                             const string & exception,
                             int exit_code,
                             const string & output,
                             const string & error_output
    ) {
        if (!exception.empty()) {
            //LOG_WARN << "exec dpkg-query command failed: " << exception;
            cout << "exec dpkg-query command failed: " << exception << endl;
            return;
        }
        this->lines.clear();
        boost::algorithm::split(this->lines, output,
                                boost::algorithm::is_any_of("\n"));
        this->it = this->lines.begin();
    });
    task->Run();
}

SoftwareInfoListPrivate::~SoftwareInfoListPrivate()
{
    lines.clear();
}

vector<string> SoftwareInfoListPrivate::MakeDpkgQueryArgs(const string &name)
{
    vector<string> args;
    args.push_back("-f");
    args.push_back("${Package},${Version},${Architecture}\n");
    args.push_back("-W");
    if (!name.empty()) {
        args.push_back(name);
    }
    return args;
}

void ParseCMDLine(string &line, SoftwareInfo *info)
{
    namespace al = boost::algorithm;
    al::trim_right_if(line, al::is_any_of("\n"));
    cout<<"CMD output: "<<line<<"-"<<endl;
    vector<string> list;
    al::split(list, line, al::is_any_of(CMDLINE_DELIM));
    if (list.size() != 3) {
        // invalid data
        //LOG_WARN << "invalid cmd line";
        cout << "invalid cmd line" << endl;
        return;
    }

    info->name = list[0];
    info->version = list[1];
    info->architecture = list[2];
}
} // namespace software
} // namespace module
} // namespace dmcg
