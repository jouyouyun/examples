#pragma once

#include <ios>
#include <vector>
#include <memory>

namespace dmcg
{
namespace module
{
namespace systeminfo
{
typedef struct _NetworkDeviceInfo {
    std::string iface;
    std::string macaddress;
    std::string address;
} NetDevInfo;

class NetworkDevicePrivate;

class NetworkDevice
{
public:
    NetworkDevice();
    ~NetworkDevice();
    std::vector<NetDevInfo> GetNetworkDeviceList();
private:
    std::unique_ptr<NetworkDevicePrivate> d;
};
} // namespace systeminfo
} // namespace module
} // namespace dmcg
