#pragma once

#include <ios>
#include <memory>

namespace dmcg
{
namespace module
{
namespace systeminfo
{
class DMIPrivate;

class DMI
{
public:
    DMI();
    ~DMI();

    std::string GetBiosDate();
    std::string GetBiosVendor();
    std::string GetBiosVersion();

    std::string GetBoardName();
    std::string GetBoardVendor();
    std::string GetBoardVersion();
    std::string GetBoardSerial();
    std::string GetBoardAssetTag();

    std::string GetProductName();
    std::string GetProductFamily();
    std::string GetProductVersion();
    std::string GetProductSerial();
    std::string GetProductUUID();
private:
    std::unique_ptr<DMIPrivate> d;
};
} // systeminfo
} // module
} // dmcg
