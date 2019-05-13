#pragma once

#include <ios>
#include <memory>
#include <vector>

namespace jouyouyun {
namespace network {
namespace device {
#define NET_SYSFS_DIR "/sys/class/net"
#define NET_VIRTUAL_DIR "/sys/devices/virtual/net"

typedef struct _DeviceInfo {
    std::string interface;
    std::string name;
    std::string ip;
    std::string macaddress;
} DeviceInfo;

class NetDevPrivate;

class NetworkDevice {
public:
    NetworkDevice(const std::string &sysfs_dir = NET_SYSFS_DIR,
                  const std::string &virtual_dir = NET_VIRTUAL_DIR);
    ~NetworkDevice();

    std::vector<DeviceInfo> List();
private:
    std::unique_ptr<NetDevPrivate> core;
};
} // namespace device
} // namespace network
} // namespace jouyouyun