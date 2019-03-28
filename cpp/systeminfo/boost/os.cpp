#include "os.h"
#include "file.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace dmcg
{
namespace module
{
namespace systeminfo
{
using namespace std;

class OSPrivate
{
    friend class OS;

private:
    string name;
    string version;
    string type; // only for 'deepin-version'

    string GetOSValue(string &key);
    void InitFromDeepinVersion();
    void InitFromOSRelease();
};

OS::OS()
{
    d = unique_ptr<OSPrivate>(new OSPrivate);
}

OS::~OS()
{
}

string OS::GetOSName()
{
    return d->GetOSValue(d->name);
}

string OS::GetOSVersion()
{
    return d->GetOSValue(d->version);
}

string OS::GetOSType()
{
    return d->GetOSValue(d->type);
}

string OSPrivate::GetOSValue(string &key)
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

void OSPrivate::InitFromDeepinVersion()
{
    // deepin
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("/etc/deepin-version", pt);
    name = "Deepin";
    version = pt.get("Release.Version", "");
    type = pt.get("Release.Type", "");
}

void OSPrivate::InitFromOSRelease()
{
    // systemd
    FilePrivate f("/etc/os-release");
    name = f.GetKey("NAME", "=");
    version = f.GetKey("VERSION", "=");
}
} // namespace systeminfo
} // namespace module
} // namesapce dmcg
