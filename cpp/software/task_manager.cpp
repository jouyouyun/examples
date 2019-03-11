#include "task_manager.h"

#include "task.h"

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

boost::shared_ptr<Task> TaskManager::Create(const std::string &program, const std::string &script)
{
    return boost::shared_ptr<Task>(new Task(program, {"-c", script}));
}

} // namespace runner
} // namespace module
} // namespace dmcgContent
