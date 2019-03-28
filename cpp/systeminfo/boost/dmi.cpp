#include "dmi.h"
#include "file.h"

#include <boost/algorithm/string.hpp>

//#include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace systeminfo
{
#define DMI_PREFIX_DIR "/sys/class/dmi/id/"

using namespace std;

// DMI info from /sys/class/dmi/id/
class DMIPrivate
{
    friend class DMI;

    // bios
    string bios_date;
    string bios_vendor;
    string bios_version;
    // board
    string board_name;
    string board_vendor;
    string board_version;
    string board_serial;
    string board_asset_tag;
    // product
    string product_name;
    string product_family;
    string product_version;
    string product_serial;
    string product_uuid;

    void LoadFileContent(const string &key,
                         string &value, bool &visited);
};

DMI::DMI()
{
    d = unique_ptr<DMIPrivate>(new DMIPrivate);
}

DMI::~DMI()
{
}

string DMI::GetBiosDate()
{
    static bool visited = false;
    d->LoadFileContent("bios_date", d->bios_date, visited);
    return d->bios_date;
}

string DMI::GetBiosVendor()
{
    static bool visited = false;
    d->LoadFileContent("bios_vendor", d->bios_vendor, visited);
    return d->bios_vendor;
}

string DMI::GetBiosVersion()
{
    static bool visited = false;
    d->LoadFileContent("bios_version", d->bios_version, visited);
    return d->bios_version;
}

string DMI::GetBoardName()
{
    static bool visited = false;
    d->LoadFileContent("board_name", d->board_name, visited);
    return d->board_name;
}

string DMI::GetBoardVendor()
{
    static bool visited = false;
    d->LoadFileContent("board_vendor", d->board_vendor, visited);
    return d->board_vendor;
}

string DMI::GetBoardVersion()
{
    static bool visited = false;
    d->LoadFileContent("board_version", d->board_version, visited);
    return d->board_version;
}

string DMI::GetBoardSerial()
{
    static bool visited = false;
    d->LoadFileContent("board_serial", d->board_serial, visited);
    return d->board_serial;
}

string DMI::GetBoardAssetTag()
{
    static bool visited = false;
    d->LoadFileContent("board_asset_tag", d->board_asset_tag, visited);
    return d->board_asset_tag;
}

string DMI::GetProductName()
{
    static bool visited = false;
    d->LoadFileContent("product_name", d->product_name, visited);
    return d->product_name;
}

string DMI::GetProductFamily()
{
    static bool visited = false;
    d->LoadFileContent("product_family", d->product_family, visited);
    return d->product_family;
}

string DMI::GetProductVersion()
{
    static bool visited = false;
    d->LoadFileContent("product_version", d->product_version, visited);
    return d->product_version;
}

string DMI::GetProductSerial()
{
    static bool visited = false;
    d->LoadFileContent("product_serial", d->product_serial, visited);
    return d->product_serial;
}

string DMI::GetProductUUID()
{
    static bool visited = false;
    d->LoadFileContent("product_uuid", d->product_uuid, visited);
    return d->product_uuid;
}

void DMIPrivate::LoadFileContent(const string &key,
                                 string &value, bool &visited)
{
    if (visited) {
        return;
    }
    FilePrivate file(DMI_PREFIX_DIR + key);
    value = file.LoadContent();
    boost::algorithm::trim_right_if(value,
                                    boost::algorithm::is_any_of("\n"));
    visited = true;
    return;
}
} // namespace systeminfo
} // namespace module
} // namespace dmcg
