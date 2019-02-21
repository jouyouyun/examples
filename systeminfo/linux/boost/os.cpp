#include "os.hpp"
#include "file.hpp"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace dmcg{
    namespace module {
        namespace systeminfo {
            OS::OS()
            {
            }

            OS::~OS()
            {
            }

            string OS::GetOSName()
            {
                return GetOSValue(name);
            }

            string OS::GetOSVersion()
            {
                return GetOSValue(version);
            }

            string OS::GetOSType()
            {
                return GetOSValue(type);
            }

            string OS::GetOSValue(string &key)
            {
                if (!name.empty() || !version.empty() || !type.empty()) {
                    return key;
                }

                if (boost::filesystem::exists("/etc/deepin-version")) {
                    InitFromDeepinVersion();
                    return key;
                }
                InitFromOSRelease();
                return key;
            }

            void OS::InitFromDeepinVersion()
            {
                // deepin
                boost::property_tree::ptree pt;
                boost::property_tree::ini_parser::read_ini("/etc/deepin-version", pt);
                name = "Deepin";
                version = pt.get("Release.Version", "");
                type = pt.get("Release.Type", "");
            }

            void OS::InitFromOSRelease()
            {
                // systemd
                FilePrivate f("/etc/os-release");
                name = f.GetKey("NAME", "=");
                version = f.GetKey("VERSION", "=");
            }
        } // namespace systeminfo
    } // namespace module
} // namesapce dmcg
