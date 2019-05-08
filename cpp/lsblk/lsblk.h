#pragma once

#include <ios>
#include <vector>
#include <memory>
#include <boost/algorithm/string.hpp>

namespace jouyouyun {
namespace disk {
namespace lsblk {
typedef struct _LsblkInfo {
    std::string name;
    std::string serial;
    std::string type;
    std::string size;
    std::string vendor;
    std::string model;
    bool root;

    std::string DisplayName()
    {
        std::string display = model + "-" + serial + "-" +vendor;
        boost::algorithm::replace_all(display, " ", "_");
        return display;
    }
} LsblkInfo;

class LsblkPrivate;

class Lsblk {
public:
    Lsblk();
    ~Lsblk();

    std::vector<LsblkInfo> List();
private:
    std::unique_ptr<LsblkPrivate> core;
};

} // namespace lsblk
} // namespace disk
} // namespace jouyouyun