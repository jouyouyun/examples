#include "executor.h"

#include <errno.h>
#include <string.h>
#include <boost/thread.hpp>

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

void RealExecute(const string &cmd, bool *running,
                 int *status, string *error);

class ExecutorPrivate
{
    friend class Executor;
public:
    ExecutorPrivate(Executor *o);
    ~ExecutorPrivate();
private:
    Executor *owner;
    bool running;
};

Executor::Executor(const string &cmd): status(EXEC_STATUS_UNKNOWN),
    cmdline(cmd)
{
    d = unique_ptr<ExecutorPrivate>(new ExecutorPrivate(this));
}

Executor::~Executor()
{
}

void Executor::Start()
{
    if (cmdline.empty()) {
        return;
    }
    d->running = true;
    boost::thread(RealExecute, d->owner->cmdline,
                  &(d->running), &(d->owner->status), &(d->owner->error));
}

bool Executor::Running()
{
    return d->running;
}

ExecutorPrivate::ExecutorPrivate(Executor *o): owner(o),
    running(false)
{
}

ExecutorPrivate::~ExecutorPrivate()
{}

void RealExecute(const string &cmd,
                 bool *running, int *status, string *error)
{
    // TODO(jouyouyun): find other way to execute command
    int ret = system(cmd.c_str());
    *running = false;
    if (ret != 0) {
        *status = EXEC_STATUS_FAILED;
        *error = string(strerror(errno));
        return;
    }
    *status = EXEC_STATUS_SUCCESS;
}
} // namespace software
} // namespace module
} // namespace dmcg
