#pragma once

#include <ios>
#include <memory>

namespace dmcg
{
namespace module
{
namespace software
{
class SoftwareInfoListPrivate;

class SoftwareInfo
{
public:
    SoftwareInfo();
    SoftwareInfo(const std::string &name);
    ~SoftwareInfo();

    std::string name;
    std::string version;
    std::string architecture;
};

class SoftwareInfoList
{
public:
    SoftwareInfoList();
    SoftwareInfoList(const std::string &name);
    ~SoftwareInfoList();
    SoftwareInfo *Next();
private:
    std::unique_ptr<SoftwareInfoListPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
