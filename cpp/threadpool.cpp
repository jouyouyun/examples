// See: https://blog.csdn.net/carbon06/article/details/78171588
//
// Compile: g++ -Wall -g threadpool.cpp -lboost_thread -lboost_system -lpthread

#include <string.h>
#include <unistd.h>
#include <iostream>

#include <boost/threadpool.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace std;

void test_threadpool();
void test_ioservice();

// 最佳线程数目 = （（线程等待时间+线程CPU时间）/线程CPU时间 ）* CPU数目
int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout<<"Usage: "<<argv[0]<<" <test(tp or io)>"<<endl;
        return 0;
    }

    if (strcmp(argv[1], "tp") == 0) {
        test_threadpool();
    } else if (strcmp(argv[1], "io") == 0) {
        test_ioservice();
    } else {
        cout<<"Invalid value"<<endl;
        return -1;
    }
    return 0;
}

// Using library 'ThreadPool'
// See: http://threadpool.sourceforge.net/
void task1()
{
    cout<<"Task 1"<<endl;
    sleep(5);
    cout<<"Task 1 done"<<endl;
}

void task2()
{
    cout<<"Task2"<<endl;
    sleep(5);
    cout<<"Task 2 done"<<endl;
}

void println(const string &line)
{
    cout<<line<<endl;
}

void test_threadpool()
{
    int n = boost::thread::hardware_concurrency();
    cout<<"CPU Thread Number: "<<n<<endl;

    namespace btp = boost::threadpool;
    btp::pool  tp(n*2);
    cout<<"Thread pool created..."<<endl;
    sleep(5);
    cout<<"Start schedule..."<<endl;
    tp.schedule(task1);
    tp.schedule(task2);
    // with args
    tp.schedule(boost::bind(println, "Hello, world"));
    for (int i = 0; i < 10; i++) {
        tp.schedule(task1);
        cout<<"Add task id: "<<i<<endl;
    }
    tp.wait();
}
// ============= ThreadPool END =====================

// using io_service && thread implement threadpool
void test_ioservice()
{
    boost::asio::io_service ioService;
    boost::thread_group thrdPool;
    // 声明一个 work 的原因是为了保证ioService的run方法在这个work销毁
    // 之前不会退出
    boost::asio::io_service::work work(ioService);
    int n = boost::thread::hardware_concurrency();

    // init thread group
    for (int i = 0; i < n*2; i++) {
        thrdPool.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
    }

    // add task
    ioService.post(boost::bind(task1));
    ioService.post(boost::bind(task2));
    ioService.post(boost::bind(println, "Hello, IO Service"));
    for (int i = 0; i < 10; i++) {
        if (i % 2 ==0) {
            ioService.post(boost::bind(task2));
        } else {
            ioService.post(boost::bind(task1));
        }
        cout<<"Add task id: "<<i<<endl;
    }

    sleep(15);
    // thrdPool.interrupt_all();
    // ioService 在stop 之后，post到ioService中的task 都不会被执行
    ioService.stop();
    thrdPool.join_all();
}
// ============= io_service END =====================
