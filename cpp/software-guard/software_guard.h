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
class SoftwareGuardPrivate;

class SoftwareGuard: public NetlinkProc
{
public:
    SoftwareGuard();
    SoftwareGuard(const std::string &blacklist_file);
    ~SoftwareGuard();

    void ReloadBlacklist(const std::string &filename);
    void AppendBlacklist(const std::string &filename);
    void HandleExecEvent(int pid);
    void Loop();
    void Quit();
private:
    std::unique_ptr<SoftwareGuardPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
