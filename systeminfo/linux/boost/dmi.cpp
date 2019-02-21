#include "dmi.hpp"
#include "file.hpp"

#include <boost/algorithm/string.hpp>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            #define DMI_PREFIX_DIR "/sys/class/dmi/id/"

            DMI::DMI()
            {
            }

            DMI::~DMI()
            {
            }

            string DMI::GetBiosDate()
            {
                static bool visited = false;
                LoadFileContent("bios_date", bios_date, visited);
                return bios_date;
            }

            string DMI::GetBiosVendor()
            {
                static bool visited = false;
                LoadFileContent("bios_vendor", bios_vendor, visited);
                return bios_vendor;
            }

            string DMI::GetBiosVersion()
            {
                static bool visited = false;
                LoadFileContent("bios_version", bios_version, visited);
                return bios_version;
            }

            string DMI::GetBoardName()
            {
                static bool visited = false;
                LoadFileContent("board_name", board_name, visited);
                return board_name;
            }

            string DMI::GetBoardVendor()
            {
                static bool visited = false;
                LoadFileContent("board_vendor", board_vendor, visited);
                return board_vendor;
            }

            string DMI::GetBoardVersion()
            {
                static bool visited = false;
                LoadFileContent("board_version", board_version, visited);
                return board_version;
            }

            string DMI::GetBoardSerial()
            {
                static bool visited = false;
                LoadFileContent("board_serial", board_serial, visited);
                return board_serial;
            }

            string DMI::GetBoardAssetTag()
            {
                static bool visited = false;
                LoadFileContent("board_asset_tag", board_asset_tag, visited);
                return board_asset_tag;
            }

            string DMI::GetProductName()
            {
                static bool visited = false;
                LoadFileContent("product_name", product_name, visited);
                return product_name;
            }

            string DMI::GetProductFamily()
            {
                static bool visited = false;
                LoadFileContent("product_family", product_family, visited);
                return product_family;
            }

            string DMI::GetProductVersion()
            {
                static bool visited = false;
                LoadFileContent("product_version", product_version, visited);
                return product_version;
            }

            string DMI::GetProductSerial()
            {
                static bool visited = false;
                LoadFileContent("product_serial", product_serial, visited);
                return product_serial;
            }

            string DMI::GetProductUUID()
            {
                static bool visited = false;
                LoadFileContent("product_uuid", product_uuid, visited);
                return product_uuid;
            }

            void DMI::LoadFileContent(const string &key,
                                      string& value, bool& visited)
            {
                if (visited) {
                    return;
                }
                FilePrivate file(DMI_PREFIX_DIR+key);
                value = file.LoadContent();
                boost::algorithm::trim_right_if(value,
                                                boost::algorithm::is_any_of("\n"));
                visited = true;
                return;
            }
        } // namespace systeminfo
    } // namespace module
} // namespace dmcg
