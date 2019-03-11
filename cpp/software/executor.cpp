#include "executor.h"

#include <errno.h>
#include <string.h>
#include <vector>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>

#include "task.h"
#include "task_manager.h"
#include <iostream>
//#include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

void RealExecute(const string &cmd, bool *running,
                 int *status, string *error);
void SplitCmdline(string cmd, string &program, vector<string> &args);

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
    namespace drunner = dmcg::module::runner;
    drunner::TaskManager manager;
    boost::shared_ptr<drunner::Task> task;
    string program;
    vector<string> args;

    //SplitCmdline(cmd, program, args);
    //if (program.empty()) {
    if (cmd.empty()) {
        *status = EXEC_STATUS_FAILED;
        *error = "program empty";
        return;
    }

    program = "/bin/sh";
    args.push_back("-c");
    args.push_back(cmd);
    task = manager.Create(program, args);
    task->Finish.connect([running, status, error](
                             const string & exception,
                             int exit_code,
                             const string & output,
    const string & error_output) {
        if (!exception.empty()) {
            *status = EXEC_STATUS_FAILED;
            *error = exception;
            *running = false;
            //LOG_WARN << "exec failed: " << exception;
            cout << "exec failed: " << exception << endl;
            return;
        }
        *status = EXEC_STATUS_SUCCESS;
        *running = false;
        cout << "Stdout: " << output << endl;
    });
    task->Run();
}

void SplitCmdline(string cmd, string &program, vector<string> &args)
{
    vector<string> items;
    boost::algorithm::split(items, cmd, boost::algorithm::is_any_of(" "));
    if (items.size() == 0) {
        return;
    } else if (items.size() == 1) {
        program = string(items.begin()->data());
        return;
    }

    vector<string>::iterator it = items.begin();
    program = string(it->data());
    args.assign(++it, items.end());
    return;
}

} // namespace software
} // namespace module
} // namespace dmcg
