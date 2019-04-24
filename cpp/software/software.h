#pragma once

#include <ios>
#include <vector>
#include <memory>

#include "software_info.h"
#include "executor.h"

namespace dmcg
{
namespace module
{
namespace software
{
class Software
{
public:
    Software();
    ~Software();

    SoftwareInfo *Get(const std::string &name);

    SoftwareInfoList *GetListIter();
    std::vector<std::unique_ptr<SoftwareInfo>> GetList();
    Executor *InstallPackage(const std::string &name);
    Executor *RemovePackage(const std::string &name);
};
} // namespace software
} // namespace module
} // namespace dmcg
