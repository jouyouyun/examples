#pragma once

#include <ios>
#include <vector>
#include <map>
#include <memory>

namespace dmcg
{
namespace module
{
namespace systeminfo
{
class LsblkPrivate;

class LsblkPartition
{
public:
    LsblkPartition(const std::string &line);
    LsblkPartition(const LsblkPartition &part);
    ~LsblkPartition();

    std::string GetPath();
    std::string GetType();
    std::string GetMountPoint();
    std::int64_t GetSize();
private:
    std::string path;
    std::string type;
    std::string mountpoint;
    std::int64_t size;

    void SplitLine(const std::string &line,
                   std::vector<std::string> &items);
};

class Lsblk
{
public:
    Lsblk();
    ~Lsblk();

    LsblkPartition *Get(const std::string &path);
    LsblkPartition *GetByMountPoint(const std::string &mountpoint);
private:
    std::unique_ptr<LsblkPrivate> d;
};
} // namespace systeminfo
} // namespace module
} // namespace dmcg
