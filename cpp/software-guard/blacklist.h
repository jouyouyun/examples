#pragma once

#include <ios>
#include <memory>

namespace dmcg
{
namespace module
{
namespace software
{
class BlacklistPrivate;

class Blacklist
{
public:
    Blacklist();
    Blacklist(const std::string &filename);
    ~Blacklist();

    void SetBlacklist(const std::string &filename, bool append);
    bool IsInList(const std::string &name);
private:
    std::unique_ptr<BlacklistPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
