#include "lsblk.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include "task.h"
#include "task_manager.h"
//#include "modules/runner/task.h"
//#include "modules/runner/task_manager.h"
//#include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace systeminfo
{
using namespace std;

#define LSBLK_LINE_MIN_NUM 6

#define PARTITION_TYPE_DISK "disk"
#define PARTITION_TYPE_PART "part"

class LsblkPrivate
{
    friend class Lsblk;

public:
    LsblkPrivate();
    ~LsblkPrivate();

private:
    void GetLsblkStdout(vector<string> &lines);
    map<string, string> part_disk_sets;
    vector<LsblkPartition> part_list;
};

Lsblk::Lsblk()
{
    d = unique_ptr<LsblkPrivate>(new LsblkPrivate);
}

Lsblk::~Lsblk()
{
}

LsblkPartition *Lsblk::Get(const string &path)
{
    if (d->part_list.size() == 0) {
        return NULL;
    }
    vector<LsblkPartition>::iterator it = d->part_list.begin();
    for (; it != d->part_list.end(); it++) {
        if (it->GetPath() == path) {
            break;
        }
    }
    if (it == d->part_list.end()) {
        return NULL;
    }

    LsblkPartition *ret = new LsblkPartition((LsblkPartition)(*it));
    return ret;
}

LsblkPartition *Lsblk::GetByMountPoint(const string &mountpoint)
{
    if (d->part_list.size() == 0 || mountpoint.empty()) {
        return NULL;
    }
    vector<LsblkPartition>::iterator it = d->part_list.begin();
    for (; it != d->part_list.end(); it++) {
        if (it->GetMountPoint() == mountpoint) {
            break;
        }
    }
    if (it == d->part_list.end()) {
        return NULL;
    }

    LsblkPartition *ret = new LsblkPartition((LsblkPartition)(*it));
    return ret;
}

LsblkPrivate::LsblkPrivate()
{
    vector<string> lines;
    GetLsblkStdout(lines);
    if (lines.size() == 0) {
        // failed
        return ;
    }

    vector<string>::iterator iter = lines.begin();
    string cur_disk;
    for (; iter != lines.end(); iter++) {
        LsblkPartition part(*iter);
        part_list.push_back(part);
        if (part.GetType() == PARTITION_TYPE_DISK) {
            cur_disk = part.GetPath();
            continue;
        }
        if (cur_disk.empty()) {
            continue;
        }
        part_disk_sets.insert(
            pair<string, string>(part.GetPath(), cur_disk));
    }
}

LsblkPrivate::~LsblkPrivate()
{
    part_disk_sets.clear();
    part_list.clear();
}

void LsblkPrivate::GetLsblkStdout(vector<string> &lines)
{
    namespace drunner = dmcg::module::runner;
    drunner::TaskManager manager;
    boost::shared_ptr<drunner::Task> task;
    string program;
    vector<string> args;

    program = "lsblk";
    args.push_back("-blp");
    task = manager.Create(program, args);
    task->Finish.connect([&lines](const string & exception,
                                  int exit_code,
                                  const string & output,
    const string & error_output) {
        if (!exception.empty()) {
            //LOG_ERROR << "Failed to exec lsblk: " << exception;
            cout << "Failed to exec lsblk: " << exception<<endl;
            return;
        }
        boost::algorithm::split(lines, output,
                                boost::algorithm::is_any_of("\n"));
        if (lines.size() > 0) {
            // erase 'NAME   MAJ:MIN RM         SIZE RO TYPE MOUNTPOINT'
            lines.erase(lines.begin());
        }
    });
    task->Run();
}

LsblkPartition::LsblkPartition(const string &line)
{
    if (line.empty()) {
        // failed
        return ;
    }
    string tmp = line;
    namespace al = boost::algorithm;
    al::trim_right_if(tmp, al::is_any_of("\n"));

    vector<string> items;
    SplitLine(tmp, items);
    if (items.size() < LSBLK_LINE_MIN_NUM) {
        // failed
        return;
    }

    vector<string>::iterator type_iter = items.begin() + 5;
    if (*type_iter != PARTITION_TYPE_DISK &&
            *type_iter != PARTITION_TYPE_PART) {
        // unsupported type
        return;
    }
    path = *(items.begin());
    type = *type_iter;
    if (items.size() == 7) {
        mountpoint = *(items.begin() + 6);
    }
    size = (int64_t)stoll(*(items.begin() + 3));
}

LsblkPartition::LsblkPartition(const LsblkPartition &part)
{
    path = part.path;
    type = part.type;
    mountpoint = part.mountpoint;
    size = part.size;
}

LsblkPartition::~LsblkPartition()
{}

string LsblkPartition::GetPath()
{
    return path;
}

string LsblkPartition::GetType()
{
    return type;
}

string LsblkPartition::GetMountPoint()
{
    return mountpoint;
}

int64_t LsblkPartition::GetSize()
{
    return size;
}

void LsblkPartition::SplitLine(const string &line,
                               vector<string> &items)
{
    namespace al = boost::algorithm;
    al::split(items, line, al::is_any_of(" "));
    if (items.size() == 0) {
        return;
    }

    // filter empty item
    vector<string>::iterator iter = items.begin();
    while (iter != items.end()) {
        if (!iter->empty()) {
            iter++;
            continue;
        }
        iter = items.erase(iter);
    }
}
} // namespace systeminfo
} // namespace module
} // namespace dmcg
