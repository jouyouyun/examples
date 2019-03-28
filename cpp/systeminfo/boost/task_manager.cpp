#include "task_manager.h"

#include "task.h"

#include <boost/filesystem/fstream.hpp>

#include <boost/process.hpp>
namespace dmcg
{
namespace module
{
namespace runner
{

class TaskManagerPrivate
{
    friend class TaskManager;

    TaskManagerPrivate(TaskManager *o)
        : owner(o)
    {}

    TaskManager *owner;
};

TaskManager::TaskManager()
    : d(new TaskManagerPrivate(this))
{}

TaskManager::~TaskManager() {}

boost::shared_ptr<Task> TaskManager::Create(const std::string &program, const std::vector<std::string> &args)
{
    return boost::shared_ptr<Task>(new Task(program, args));
}

boost::shared_ptr<Task> TaskManager::Create(const std::string &program, const std::string &script, const std::vector<std::string> &args)
{
    std::vector<std::string> scriptArgs;
    scriptArgs.push_back("-c");
    scriptArgs.push_back(script);

    scriptArgs.insert(scriptArgs.end(), args.begin(), args.end());
    return boost::shared_ptr<Task>(new Task(program, scriptArgs));
}

} // namespace runner
} // namespace module
} // namespace dmcgContent
