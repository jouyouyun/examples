/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * task_timer.cpp -- A task timer example
 *
 * Compile: g++ -Wall -g task_timer.cpp -lboost_system -lpthread
 *
 * Written on 星期五, 26 四月 2019.
 */

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;

void onDuration(boost::asio::deadline_timer *timer, const string &msg)
{
    cout<<msg<<" recieved!"<<endl;
    if (!timer) {
        return;
    }
    boost::posix_time::seconds duration(5);
    timer->expires_at(timer->expires_at() + duration);
    timer->async_wait(boost::bind(onDuration, timer, msg));
}

int main()
{
    boost::asio::io_service io;
    boost::posix_time::seconds duration1(5);
    boost::posix_time::seconds duration2(2);
    boost::asio::deadline_timer timer1(io, duration1);
    boost::asio::deadline_timer timer2(io, duration2);

    timer1.async_wait(boost::bind(onDuration, &timer1, "Duration1"));
    timer2.async_wait(boost::bind(onDuration, &timer2, "Duration2"));

    io.run();
    return 0;
}
