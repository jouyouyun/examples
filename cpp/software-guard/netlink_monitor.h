#pragma once

#include "netlink_proc.h"

#include <ios>
#include <memory>

namespace dmcg
{
namespace module
{
namespace software
{
class NLMonitorPrivate;
class NetlinkMonitor
{
public:
    NetlinkMonitor(NetlinkProc *proc);
    ~NetlinkMonitor();

    int Loop();
    void Quit();
private:
    std::unique_ptr<NLMonitorPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
