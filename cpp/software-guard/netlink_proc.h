#pragma once

namespace dmcg
{
namespace module
{
namespace software
{
class NetlinkProc
{
public:
    virtual void HandleExecEvent(int pid) = 0;
};
} // namespace software
} // namespace module
} // namespace dmcg
