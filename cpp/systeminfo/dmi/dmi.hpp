#ifndef __MACHINE_DMI_H__
#define __MACHINE_DMI_H__

#include <iostream>

using namespace std;

// DMI info from /sys/class/dmi/id/
class DMI{
public:
    DMI();
    ~DMI();

    string GetBiosDate();
    string GetBiosVendor();
    string GetBiosVersion();

    string GetBoardName();
    string GetBoardVendor();
    string GetBoardVersion();
    string GetBoardSerial();
    string GetBoardAssetTag();

    string GetProductName();
    string GetProductFamily();
    string GetProductVersion();
    string GetProductSerial();
    string GetProductUUID();
private:
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

    string GetFileContents(const string &filename);
};

#endif
