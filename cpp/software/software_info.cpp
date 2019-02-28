#include "software_info.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

#define MAX_BUF_SIZE 1024
#define CMDLINE "dpkg-query -f '${Package},${Version}\n' -W"
#define CMDLINE_DELIM ","

class SoftwareInfoListPrivate
{
    friend class SoftwareInfoList;
public:
    SoftwareInfoListPrivate(const string &cmd);
    ~SoftwareInfoListPrivate();
private:
    FILE *fp;
    char buf[MAX_BUF_SIZE];
};

void ParseCMDLine(string &line, SoftwareInfo *info);

SoftwareInfo::SoftwareInfo()
{
}

SoftwareInfo::~SoftwareInfo()
{
}

SoftwareInfo::SoftwareInfo(const string &name)
{
    string cmd = string(CMDLINE) + string(" ") + name;
    FILE *fp = NULL;
    char buf[MAX_BUF_SIZE] = {0};

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        cout << "exec dkpg-query command failed" << endl;
        return;
    }

    if (fgets(buf, MAX_BUF_SIZE, fp) == NULL) {
        pclose(fp);
        cout << "no output found" << endl;
        return;
    }
    pclose(fp);
    string line(buf);
    ParseCMDLine(line, this);
}

SoftwareInfoList::SoftwareInfoList()
{
    d = unique_ptr<SoftwareInfoListPrivate>(new SoftwareInfoListPrivate(CMDLINE));
}

SoftwareInfoList::~SoftwareInfoList()
{}

SoftwareInfo *SoftwareInfoList::Next()
{
    if (!d->fp || feof(d->fp)) {
        return NULL;
    }
    memset(d->buf, 0, MAX_BUF_SIZE);
    fgets(d->buf, MAX_BUF_SIZE, d->fp);

    string line = string(d->buf);
    SoftwareInfo *info = new SoftwareInfo;
    ParseCMDLine(line, info);
    return info;
}

SoftwareInfoListPrivate::SoftwareInfoListPrivate(const string &cmd)
{
    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        // Error
        cout << "exec dpkg-query failed" << endl;
    }
}

SoftwareInfoListPrivate::~SoftwareInfoListPrivate()
{
    if (fp) {
        pclose(fp);
    }
}

void ParseCMDLine(string &line, SoftwareInfo *info)
{
    namespace al = boost::algorithm;
    al::trim_right_if(line, al::is_any_of("\n"));
    vector<string> list;
    al::split(list, line, al::is_any_of(CMDLINE_DELIM));
    if (list.size() != 2) {
        // invalid data
        cout << "invalid cmd line" << endl;
        return;
    }

    info->name = list[0];
    info->version = list[1];
}
}
}
}
