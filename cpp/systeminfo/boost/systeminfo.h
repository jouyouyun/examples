#pragma once

#include "os.h"
#include "dmi.h"
#include "network_device.h"

namespace dmcg
{
namespace module
{
namespace systeminfo
{
class SystemInfoPrivate;

// TODO: load by interface

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

class SystemInfo
{
public:
    std::string GetMachineID() {return "ff1cd6ed-2d89-495f-9caf-cd6ce65d0179";}
    std::string GetCPU() {return "";}
    std::int64_t GetMemoryCap() {return 0;}
    std::int64_t GetDiskCap() {return 0;}
};

#else

class SystemInfo: public OS, public DMI, public NetworkDevice
{
public:
    SystemInfo();
    ~SystemInfo();

    std::string GetMachineID();
    std::string GetCPU();
    std::int64_t GetMemoryCap();
    std::int64_t GetDiskCap();
private:
    std::unique_ptr<SystemInfoPrivate> d;
};

#endif

} // namespace systeminfo
} // namespace module
} // namespace dmcg
