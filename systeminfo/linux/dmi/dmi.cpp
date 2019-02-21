#include "dmi.hpp"
#include "file.hpp"

#define DMI_PREFIX_DIR "/sys/class/dmi/id/"

DMI::DMI()
{
}

DMI::~DMI()
{}

string DMI::GetBiosDate()
{
    static bool visited = false;
    if (visited) {
        return bios_date;
    }
    bios_date = GetFileContents("bios_data");
    visited = true;
    return bios_date;
}

string DMI::GetBiosVendor()
{}

string DMI::GetBiosVersion()
{}

string DMI::GetBoardName()
{}

string DMI::GetBoardVendor()
{}

string DMI::GetBoardVersion()
{}

string DMI::GetBoardSerial()
{}

string DMI::GetBoardAssetTag()
{}

string DMI::GetProductName()
{}

string DMI::GetProductFamily()
{}

string DMI::GetProductVersion()
{}

string DMI::GetProductSerial()
{}

string DMI::GetProductUUID()
{}

string DMI::GetFileContents(const string& filename)
{
    File f(DMI_PREFIX_DIR + filename, ios::in);
    string contents = f.GetContents();
    return contents;
}
