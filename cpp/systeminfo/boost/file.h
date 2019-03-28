#pragma once

#include <ios>
#include <fstream>

namespace dmcg
{
namespace module
{
namespace systeminfo
{
// DMI info from /sys/class/dmi/id/
class FilePrivate
{
public:
    FilePrivate(const std::string &filepath,
                std::ios::openmode mode = std::ios::in);
    ~FilePrivate();
    std::string LoadContent();
    std::string GetKey(const std::string &key, const std::string &delim);
private:
    std::fstream *fp;
};
} // namespace systeminfo
} // namespace module
} // namesapce dmcg
