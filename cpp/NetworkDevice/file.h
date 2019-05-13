#pragma once

#include <ios>
#include <memory>
#include <vector>
#include <fstream>

namespace jouyouyun
{
namespace network
{
namespace device
{
class KeyValueFile
{
public:
    KeyValueFile(const std::string &filepath,
                 const std::string &sep = "=",
                 std::ios::openmode mode = std::ios::in);
    ~KeyValueFile();

    std::string LoadContent();
    std::string GetKey(const std::string &key);
private:
    std::string line_sep;
    std::unique_ptr<std::fstream> core;
};

std::vector<std::string> SplitString(const std::string &str,
                                     const std::string &sep);
} // namespace device
} // namespace network
} // namesapce jouyouyun
