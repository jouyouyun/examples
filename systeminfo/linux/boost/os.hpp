#ifndef __MODULE_SYSTEMINFO_OS_H__
#define __MODULE_SYSTEMINFO_OS_H__

#include <ios>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            using namespace std;
            class OS {
            public:
                OS();
                ~OS();

                string GetOSName();
                string GetOSVersion();
                string GetOSType();
            private:
                string name;
                string version;
                string type; // only for 'deepin-version'

                string GetOSValue(string& key);
                void InitFromDeepinVersion();
                void InitFromOSRelease();
            };
        } // namespace systeminfo
    } // namespace module
} // namespace dmcg

#endif
