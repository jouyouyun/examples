#pragma once

#include <ios>
#include <memory>

namespace jouyouyun {
namespace example {
namespace unittest {
#define HOSTNAME_FILE "/etc/hostname"

class HostnamePrivate;

class Hostname {
public:
    Hostname(const std::string &file = HOSTNAME_FILE);
    ~Hostname();

    std::string Get();
    int Set(const std::string &name);

private:
    std::unique_ptr<HostnamePrivate> core;
};
} // namespace unittest
} // namespace example
} // namesapce jouyouyun