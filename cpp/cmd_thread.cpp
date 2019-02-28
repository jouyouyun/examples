#include <iostream>
#include <boost/thread.hpp>
#include <stdio.h>

using namespace std;

void cmd_runner(const string& cmd)
{
    // try {
    // NOT TERMINATED
    FILE *fp = popen(cmd.c_str(), "r");
    cout<<"Runner open: "<<fp<<endl;
    pclose(fp);
    cout<<"Runner over"<<endl;
    // } catch (boost::thread_interrupted& interrupt) {
        // cout<<"CMD Interrupt"<<endl;
        // return ;
    // }
}

void loop_runner()
{
    static int count = 1;
    // try {
        for (;;count++) {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
            cout<<"Loop: "<<count<<endl;
        }
    // } catch(boost::thread_interrupted&){
        // cout<<"Loop Interrupt"<<endl;
    // }
}

void do_intterrupt(boost::thread* t)
{
    cout<<"do intterrupt"<<endl;
    boost::this_thread::sleep(boost::posix_time::seconds(3));
    cout<<"Raise intterupt"<<endl;
    (*t).interrupt();
}

int main()
{
    boost::thread *t1 = new boost::thread(cmd_runner, "sleep 10 && echo 'Test Thread' >> /tmp/thread.log");
    // boost::thread *t1 = new boost::thread(loop_runner);
    boost::thread t2(do_intterrupt, t1);

    cout<<"Join"<<endl;
    // t1->join();
    // t2.join();
    boost::this_thread::sleep(boost::posix_time::seconds(30));
    return 0;
}
