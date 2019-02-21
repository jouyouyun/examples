#ifndef __MODULE_SYSTEMINFO__LSBLK_H__
#define __MODULE_SYSTEMINFO__LSBLK_H__

#include <iostream>
#include <vector>
#include <map>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            using namespace std;

            class LsblkPartition {
            public:
                LsblkPartition(const string& line);
                LsblkPartition(const LsblkPartition& part);
                ~LsblkPartition();

                string GetPath();
                string GetType();
                string GetMountPoint();
                int64_t GetSize();
            private:
                string path;
                string type;
                string mountpoint;
                int64_t size;

                void SplitLine(const string& line, vector<string>& items);
            };

            class LsblkPrivate {
            public:
                LsblkPrivate();
                ~LsblkPrivate();

                LsblkPartition* Get(const string& path);
                LsblkPartition* GetByMountPoint(const string& mountpoint);
            private:
                void GetLsblkStdout(vector<string>& lines);
                map<string, string> part_disk_sets;
                vector<LsblkPartition> part_list;
            };
        } // namespace systeminfo
    } // namespace module
} // namespace dmcg

#endif
