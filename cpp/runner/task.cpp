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
	std::cout<<"Task destroy: "<<this<<std::endl;
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
		std::cout<<"Run "<<d->program<<std::endl;

        bp::child c(d->program, d->args,
                    bp::std_in.close(),
                    bp::std_out > out_buf,
                    bp::std_err > err_buf,
                    ios);

        ios.run(); //this will actually block until the compiler is finished

        auto output_data = out_buf.get();
        auto err_data = err_buf.get();

        // LOG_WARN << "Command output: \n"
        //          << output_data;
        // LOG_WARN << "Command error: \n"
        //          << err_data;
        //LOG_WARN << "Command exit_code: " << c.exit_code();
		std::cout<<"Command exit code: "<<c.exit_code()<<std::endl;

        this->Finish("", c.exit_code(), output_data, err_data);
    } catch (const bp::process_error &e) {
        //LOG_ERROR << "Exception with code: " << e.code() << " what: " << e.what();
		std::cout << "Exception with code: " << e.code() << " what: " << e.what()<<std::endl;
        this->Finish(e.what(), -1, "", "");
    } catch (const std::exception &e) {
        //LOG_ERROR << "Exception with what: " << e.what();
		std::cout << "Exception with what: " << e.what()<<std::endl;
        this->Finish(e.what(), -1, "", "");
    } catch (...) {
        //LOG_ERROR << "Unknow exception";
		std::cout << "Unknow exception"<<std::endl;
        this->Finish("unknow", -1, "", "");
    }
}

void Task::Run(bool joined)
{
    d->thread.reset(new boost::thread(boost::ref(*this)));
    if (joined) {
        d->thread->join();
    } else {
        d->thread->detach();
    }
}

} // namespace runner
} // namespace module
} // namespace dmcg
