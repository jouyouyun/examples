#include "task.h"

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/thread.hpp>

#include <iostream>
//#include "modules/log/log.h"

namespace bp = boost::process;

namespace dmcg
{
namespace module
{
namespace runner
{
    using namespace std;

class TaskPrivate
{
    friend class Task;

    TaskPrivate(Task *o)
        : owner(o)
    {
    }

    std::string program;
    std::vector<std::string> args;

    boost::scoped_ptr<boost::thread> thread;

    Task *owner;
};

Task::Task(const std::string &program, const std::vector<std::string> &args)
    : d(new TaskPrivate(this))
{
    d->program = program;
    d->args = args;
}

Task::~Task()
{
    //LOG_WARN << "Task destory: " << this;
    cout << "Task destory: " << this<<endl;
}

void Task::operator()()
{
    try {
        boost::asio::io_service ios;
        std::future<std::string> out_buf;
        std::future<std::string> err_buf;

        if (!boost::filesystem::exists(d->program)) {
            d->program = bp::search_path(d->program).string();
        }

        //LOG_WARN << "Run " << d->program;
        cout << "Run " << d->program<<endl;

        bp::child c(d->program, d->args,
                    bp::std_in.close(),
                    bp::std_out > out_buf,
                    bp::std_err > err_buf,
                    ios);
        ios.run();

        // WARINNGIN: exit_code need wait finish
        c.wait();

        auto output_data = out_buf.get();
        auto err_data = err_buf.get();

        // LOG_WARN << "Command output: \n"
        //          << output_data;
        // LOG_WARN << "Command error: \n"
        //          << err_data;
        //LOG_WARN << "Command exit_code: " << c.exit_code();
        cout << "Command exit_code: " << c.exit_code()<<endl;

        this->Finish("", c.exit_code(), output_data, err_data);
    } catch (const bp::process_error &e) {
        //LOG_ERROR << "Exception with code: " << e.code() << " what: " << e.what();
        cout << "Exception with code: " << e.code() << " what: " << e.what()<<endl;
        this->Finish(e.what(), -1, "", "");
    } catch (const std::exception &e) {
        //LOG_ERROR << "Exception with what: " << e.what();
        cout << "Exception with what: " << e.what()<<endl;
        this->Finish(e.what(), -1, "", "");
    } catch (...) {
        //LOG_ERROR << "Unknow exception";
        cout << "Unknow exception"<<endl;
        this->Finish("unknow", -1, "", "");
    }
}

void Task::Run()
{
    d->thread.reset(new boost::thread(boost::ref(*this)));
    d->thread->join();
}

} // namespace runner
} // namespace module
} // namespace dmcg
