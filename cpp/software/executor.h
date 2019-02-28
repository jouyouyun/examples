#pragma once

#include <ios>
#include <memory>

namespace dmcg
{
namespace module
{
namespace software
{
#define EXEC_STATUS_UNKNOWN 100
#define EXEC_STATUS_SUCCESS 101
#define EXEC_STATUS_FAILED 102

class ExecutorPrivate;

class Executor
{
public:
    Executor(const std::string &cmd);
    ~Executor();

    void Start();
    bool Running();
    // TODO(jouyouyun): add Pause/Stop

    int status;
    std::string error;
    std::string cmdline;
private:
    std::unique_ptr<ExecutorPrivate> d;
};
} // namespace software
} // namespace module
} // namespace dmcg
