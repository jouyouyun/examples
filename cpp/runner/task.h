#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/signals2.hpp>

#include <vector>

namespace dmcg
{
namespace module
{
namespace runner
{

class TaskPrivate;
class Task
{
public:
    Task(const std::string &program, const std::vector<std::string> &args);
    ~Task();

    void operator()();

    boost::signals2::signal<void(const std::string &exception, int, const std::string &, const std::string &)> Finish;

    void Run(bool joined);

private:
    boost::scoped_ptr<TaskPrivate> d;
};

} // namespace runner
} // namespace module
} // namespace dmcg
