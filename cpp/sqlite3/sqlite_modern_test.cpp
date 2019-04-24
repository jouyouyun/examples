/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.cpp -- test sqlite_modern_cpp
 *
 * Written on 星期三, 24 四月 2019.
 */

#include <string.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <sqlite_modern_cpp.h>

using namespace std;

// Tables:
//        software(
//                 id integer primary key autoincrement not null,
//                 name varchar(64) not null,
//                 version varchar(128) not null,
//                 architecture varchar(16) not null
//               )
//
//        software_path(
//                      id integer primary key autoincrement not null,
//                      filepath varchar(255) not null unique,
//                      name varchar(64) not null
//                     )

shared_ptr<sqlite::database> init_db_conn(const string &filename);
int init_db_table(shared_ptr<sqlite::database> handler);
int insert_table_software(shared_ptr<sqlite::database> handler);
int insert_table_software_path(shared_ptr<sqlite::database> handler);
int query_software(shared_ptr<sqlite::database> handler);
int query_software_path(shared_ptr<sqlite::database> handler);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cout<<"Usage: "<<argv[0]<<" <db filepath> <action: init/query>"<<endl;
        return -1;
    }
    shared_ptr<sqlite::database> handler;
    int ret = 0;

    handler = init_db_conn(argv[1]);
    if (!handler) {
        return -1;
    }

    if (strcmp(argv[2], "init") == 0) {
        ret = init_db_table(handler);
        if (ret != 0) {
            goto finish;
        }

        ret = insert_table_software(handler);
        if (ret != 0) {
            goto finish;
        }

        ret = insert_table_software_path(handler);
        if (ret != 0) {
            goto finish;
        }
    } else if (strcmp(argv[2], "query") == 0) {
        ret = query_software(handler);
        if (ret != 0) {
            goto finish;
        }

        ret = query_software_path(handler);
        if (ret != 0) {
            goto finish;
        }
    }

finish:
    return ret;
}

shared_ptr<sqlite::database>
init_db_conn(const string &filename)
{
    shared_ptr<sqlite::database> core;
    try {
        core = shared_ptr<sqlite::database>(new sqlite::database(filename));
    } catch (exception &e) {
        cout<<"Failed to init db: "<<e.what()<<endl;
        return NULL;
    }

    return core;
}

int
init_db_table(shared_ptr<sqlite::database> handler)
{
    try {
        sqlite::database *db = handler.get();
        // table 'software'
        *db<<"create table if not exists software("
           "id integer primary key autoincrement not null,"
           "name varchar(64) not null,"
           "version varchar(128) not null,"
           "architecture varchar(16) not null"
           ");";

        // table 'software_path'
        *db<<"create table if not exists software_path("
           "id integer primary key autoincrement not null,"
           "filepath varchar(64) not null unique,"
           "name varchar(64) not null"
           ");";
    } catch (exception &e) {
        cout<<"Failed to init db table: "<<e.what()<<endl;
        return -1;
    }

    return 0;
}

int
insert_table_software(shared_ptr<sqlite::database> handler)
{
    vector<string> names = {"gedit","gvim","htop","terminator"};
    vector<string> versions = {"3.22", "8.1", "2.2", "1.91"};

    try {
        sqlite::database *db = handler.get();
        *db<<"delete from software";
        for (size_t i = 0; i < names.size(); i++) {
            *db<<u"insert into software (name,version,architecture) values(?,?,?);"
               <<names[i]<<versions[i]<<"amd64";
        }
    } catch(exception &e) {
        cout<<"Failed to insert to software: "<<e.what()<<endl;
        return -1;
    }

    return 0;
}

int
insert_table_software_path(shared_ptr<sqlite::database> handler)
{
    vector<string> paths = {"/usr/bin/gedit","/usr/bin/gvim","/usr/bin/htop","/usr/bin/terminator"};
    vector<string> names = {"gedit", "gvim", "htop", "terminator"};

    try {
        sqlite::database *db = handler.get();
        *db<<"delete from software_path";
        for (size_t i = 0; i < paths.size(); i++) {
            *db<<u"insert into software_path (filepath,name) values(?,?);"
               <<paths[i]<<names[i];
        }
    } catch(exception &e) {
        cout<<"Failed to insert to software_path: "<<e.what()<<endl;
        return -1;
    }

    return 0;
}

int
query_software(shared_ptr<sqlite::database> handler)
{
    try {
        sqlite::database *db = handler.get();
        *db<<"select id,name,version,architecture from software;"
           >>[&](int id, string name, string version, string arch){
                 cout<<"id: "<<id<<", name: "<<name
                     <<", version: "<<version
                     <<", arch: "<<arch<<endl;
             };
    } catch(exception &e) {
        cout<<"Failed to query software: "<<e.what()<<endl;
        return -1;
    }

    return 0;
}

int
query_software_path(shared_ptr<sqlite::database> handler)
{
    try {
        sqlite::database *db = handler.get();
        *db<<"select id,filepath,name from software_path;"
           >>[&](int id, string path, string name){
                 cout<<"id: "<<id<<", filepath: "<<path
                     <<", name: "<<name<<endl;
             };
    } catch(exception &e) {
        cout<<"Failed to query software_path: "<<e.what()<<endl;
        return -1;
    }

    return 0;
}
