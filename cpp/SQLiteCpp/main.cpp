/**
 * Copyright (C) 2019 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.cpp -- Test SQLiteCpp library
 *
 * Written on 星期五, 26 四月 2019.
 */

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

using namespace std;

static SQLite::Database *init_db(const string &filepath);
static int init_table(shared_ptr<SQLite::Database> db_ptr);
static int insert_table_software(shared_ptr<SQLite::Database> db_ptr);
static int insert_table_software_path(shared_ptr<SQLite::Database> db_ptr);
static int query_table_software(shared_ptr<SQLite::Database> db_ptr);
static int query_table_software_path(shared_ptr<SQLite::Database> db_ptr);

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        cout<<"Usage: "<<argv[0]<<" <db file> <action: init/query>"<<endl;
        return -1;
    }

    SQLite::Database *db;
    shared_ptr<SQLite::Database> db_ptr = NULL;
    int ret = 0;

    db = init_db(argv[1]);
    if (!db) {
        return -1;
    }

    db_ptr = shared_ptr<SQLite::Database>(db);

    if (string(argv[2]).compare("init") == 0) {
        ret = init_table(db_ptr);
        if (ret != 0) {
            return -1;
        }

        ret = insert_table_software(db_ptr);
        if (ret != 0) {
            return -1;
        }
        ret = insert_table_software_path(db_ptr);
        if (ret != 0) {
            return -1;
        }
    } else if (string(argv[2]).compare("query") == 0) {
        ret = query_table_software(db_ptr);
        if (ret != 0) {
            return -1;
        }
        ret = query_table_software_path(db_ptr);
        if (ret != 0) {
            return -1;
        }
    } else {
        cout<<"Invalid action"<<endl;
        return -1;
    }

    return 0;
}

static SQLite::Database *
init_db(const string &filepath)
{
    SQLite::Database *db = NULL;
    try {
        db = new SQLite::Database(filepath, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    } catch (exception &e) {
        cout<<"Failed to open db: "<<e.what()<<endl;
        return NULL;
    }
    return db;
}

static int
init_table(shared_ptr<SQLite::Database> db_ptr)
{
    try {
        db_ptr->exec("DROP TABLE IF EXISTS software");
        db_ptr->exec("CREATE TABLE IF NOT EXISTS software("
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            "name VARCHAR(128) NOT NULL UNIQUE,"
            "version VARCHAR(128) NOT NULL,"
            "architecture VARCHAR(16) NOT NULL"
            ");");

        db_ptr->exec("DROP TABLE IF EXISTS software_path");
        db_ptr->exec("CREATE TABLE IF NOT EXISTS software_path("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                     "filepath VARCHAR(255) NOT NULL UNIQUE,"
                     "name VARCHAR(128) NOT NULL"
                     ");");
    } catch (exception &e) {
        cout<<"Failed to init table: "<<e.what()<<endl;
        return -1;
    }
    return 0;
}

static int
insert_table_software(shared_ptr<SQLite::Database> db_ptr)
{
    vector<string> names = {"gedit", "gvim", "htop"};
    vector<string> versions = {"3.22", "8.2", "1.2"};

    try {
        for (size_t i = 0; i < names.size(); i++) {
            SQLite::Statement sql(*db_ptr,
                                  "INSERT INTO software (name,version,architecture) values (?,?,?);");
            sql.bind(1, names[i]);
            sql.bind(2, versions[i]);
            sql.bind(3, "amd64");
            sql.exec();
        }
    } catch (exception &e) {
        cout<<"Failed to insert table software: "<<e.what()<<endl;
        return -1;
    }
    return 0;
}

static int
insert_table_software_path(shared_ptr<SQLite::Database> db_ptr)
{
    vector<string> paths = {"/usr/bin/gedit", "/usr/bin/gvim", "/usr/bin/htop"};
    vector<string> names = {"gedit", "gvim", "htop"};

    try {
        for (size_t i = 0; i < paths.size(); i++) {
            SQLite::Statement sql(*db_ptr,
                                  "INSERT INTO software_path (filepath,name) values (?,?);");
            sql.bind(1, paths[i]);
            sql.bind(2, names[i]);
            sql.exec();
        }
    } catch (exception &e) {
        cout<<"Failed to insert table software_path: "<<e.what()<<endl;
        return -1;
    }
    return 0;
}

static int
query_table_software(shared_ptr<SQLite::Database> db_ptr)
{
    try {
        SQLite::Statement query(*db_ptr, "SELECT id,name,version,architecture FROM software;");
        cout<<"[Software] query:"<<endl;
        while (query.executeStep()) {
            cout<<"\tID: "<<query.getColumn(0)
                <<"\tName: "<<query.getColumn(1)
                <<"\tVersion: "<<query.getColumn(2)
                <<"\tArchitecture: "<<query.getColumn(3)
                <<endl;
        }
    } catch (exception &e) {
        cout<<"Failed to query table software: "<<e.what()<<endl;
        return -1;
    }
    return 0;
}

static int
query_table_software_path(shared_ptr<SQLite::Database> db_ptr)
{
    try {
        SQLite::Statement query(*db_ptr, "SELECT id,filepath,name FROM software_path;");
        cout<<"[SoftwarePath] query:"<<endl;
        while (query.executeStep()) {
            cout<<"\tID: "<<query.getColumn(0)
                <<"\tFilepath: "<<query.getColumn(1)
                <<"\tName: "<<query.getColumn(2)
                <<endl;
        }
    } catch (exception &e) {
        cout<<"Failed to query table software: "<<e.what()<<endl;
        return -1;
    }
    return 0;
}
