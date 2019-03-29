#pragma once

#include <iostream>
#include <memory>

namespace dmcg
{
namespace module
{
namespace software
{
class SoftHistoryPrivate;

class SoftwareHistory
{
public:
    SoftwareHistory();
    ~SoftwareHistory();

    void SaveStartup(const std::string &pkg);
    void SaveShutdown(const std::string &pkg);
    std::string Dump();

private:
    std::unique_ptr<SoftHistoryPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
