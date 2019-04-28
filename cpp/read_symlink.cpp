/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * read_link.cpp -- read symlink file
 *
 * Compile: g++ -Wall -g read_link.cpp -lboost_system -lboost_filesystem
 *
 * Written on 星期日, 28 四月 2019.
 */

#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
namespace bfile = boost::filesystem;

static bfile::path get_real_path(const bfile::path &src);

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        cout<<"Usage: "<<argv[0]<<" <filepath>"<<endl;
        return -1;
    }

    bfile::path src(argv[1]);
    if (!bfile::exists(src)) {
        return -1;
    }

    bfile::path dest = get_real_path(src);

    cout<<src.string()<<" ---> "<<dest.string()<<endl;
    cout<<"\t"<<src.filename()<<" ---> "<<dest.filename()<<endl;
    return 0;
}

static bfile::path
get_real_path(const bfile::path &src)
{
    if (!bfile::is_symlink(src)) {
        return src;
    }

    bfile::path dest = bfile::read_symlink(src);
    return get_real_path(dest);
}
