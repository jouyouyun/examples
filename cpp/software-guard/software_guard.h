#pragma once

#include "netlink_proc.h"

#include <ios>
#include <memory>
#include <boost/signals2.hpp>

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
    SoftwareGuard(const std::string &whitelist_file,
                  const std::string &blacklist_file);
    ~SoftwareGuard();

    void ReloadBlacklist(const std::string &filename);
    void AppendBlacklist(const std::string &filename);
    void ReloadWhitelist(const std::string &filename);
    void AppendWhitelist(const std::string &filename);

    std::string DumpSoftwareHistory();

    void HandleExecEvent(int pid);
    void HandleExitEvent(int pid);
    void Loop();
    void Quit();

    boost::signals2::signal<void(std::string package)> Kill;

private:
    std::unique_ptr<SoftwareGuardPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
