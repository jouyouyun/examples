#ifndef __MODULES_SYSTEMINFO_FILE_H__
#define __MODULES_SYSTEMINFO_FILE_H__

#include <iostream>
#include <fstream>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            // DMI info from /sys/class/dmi/id/
            using namespace std;
            class FilePrivate {
            public:
                FilePrivate(const string& filepath, ios::openmode mode = ios::in);
                ~FilePrivate();
                string LoadContent();
                string GetKey(const string& key, const string& delim);
            private:
                fstream *fp;
            };
        } // namespace systeminfo
    } // namespace module
} // namesapce dmcg

#endif
