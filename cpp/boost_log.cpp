// 必须加上这行，不然链接会出错
// g++ -Wall -g ./boost_log.cpp -lboost_log -lpthread
#define BOOST_LOG_DYN_LINK 1

#include<boost/log/trivial.hpp>

int main(int argc, char* argv[])
{
    BOOST_LOG_TRIVIAL(trace)<<"A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    return 0;
}
