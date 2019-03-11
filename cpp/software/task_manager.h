#pragma once

#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcg
{
namespace module
{
namespace runner
{

class Task;
class TaskManagerPrivate;

class TaskManager
{
public:
    TaskManager();
    ~TaskManager();

    std::vector<boost::shared_ptr<Task>> ListTask();

    boost::shared_ptr<Task> Create(const std::string &program, const std::vector<std::string> &args);
    boost::shared_ptr<Task> Create(const std::string &program, const std::string &script);

    void CancelTask(boost::shared_ptr<Task> task);

private:
    boost::scoped_ptr<TaskManagerPrivate> d;
};

} // namespace runner
} // namespace module
} // namespace dmcg
