#pragma once

#include <ios>
#include <memory>

namespace dmcg
{
namespace module
{
namespace systeminfo
{
class OSPrivate;

class OS
{
public:
    OS();
    ~OS();

    std::string GetOSName();
    std::string GetOSVersion();
    std::string GetOSType();
private:
    std::unique_ptr<OSPrivate> d;
};
} // namespace systeminfo
} // namespace module
} // namespace dmcg
