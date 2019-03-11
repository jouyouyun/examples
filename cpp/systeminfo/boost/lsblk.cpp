#include "lsblk.hpp"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>

#include "../../runner/task_manager.h"
#include "../../runner/task.h"

namespace dmcg {
    namespace module {
        namespace systeminfo {
            #define MAX_BUF_SIZE 1024
            #define LSBLK_LINE_MIN_NUM 6

            #define PARTITION_TYPE_DISK "disk"
            #define PARTITION_TYPE_PART "part"

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
                        pair<string,string>(part.GetPath(), cur_disk));
                }
            }

            LsblkPrivate::~LsblkPrivate()
            {
                part_disk_sets.clear();
                part_list.clear();
            }

            LsblkPartition* LsblkPrivate::Get(const string &path)
            {
                if (part_list.size() == 0) {
                    return NULL;
                }
                vector<LsblkPartition>::iterator it = part_list.begin();
                for (; it != part_list.end(); it++) {
                    if (it->GetPath() == path) {
                        break;
                    }
                }
                if (it == part_list.end()) {
                    return NULL;
                }

                LsblkPartition *ret = new LsblkPartition((LsblkPartition)(*it));
                return ret;
            }

            LsblkPartition* LsblkPrivate::GetByMountPoint(const string &mountpoint)
            {
                if (part_list.size() == 0 || mountpoint.empty()) {
                    return NULL;
                }
                vector<LsblkPartition>::iterator it = part_list.begin();
                for (; it != part_list.end(); it++) {
                    if (it->GetMountPoint() == mountpoint) {
                        break;
                    }
                }
                if (it == part_list.end()) {
                    return NULL;
                }

                LsblkPartition *ret = new LsblkPartition((LsblkPartition)(*it));
                return ret;
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

            LsblkPartition::LsblkPartition(const string& line)
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

                // cout<<endl<<"-------- After line split --------"<<endl;
                // for (vector<string>::iterator it = items.begin(); it != items.end(); it++) {
                //     cout<<*it<<endl;
                // }
                // cout<<"-------- END --------"<<endl;

                vector<string>::iterator type_iter = items.begin() + 5;
                if (*type_iter != PARTITION_TYPE_DISK &&
                    *type_iter != PARTITION_TYPE_PART) {
                    // unsupported type
                    return;
                }
                path = *(items.begin());
                type = *type_iter;
                if (items.size() == 7) {
                    mountpoint = *(items.begin()+6);
                }
                size = (int64_t)stoll(*(items.begin()+3));
            }

            LsblkPartition::LsblkPartition(const LsblkPartition& part)
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

            void LsblkPartition::SplitLine(const string& line,
                                           vector<string>& items)
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
