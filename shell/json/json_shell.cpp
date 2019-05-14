// Compile: g++ -Wall -g
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

#include <boost/shared_ptr.hpp>

#include "../../cpp/runner/task.h"
#include "../../cpp/runner/task_manager.h"

using namespace std;

static string
load_filename(const string &filename)
{
    string content;
    try {
        fstream fr;
        fr.open(filename, ios::in);
        istreambuf_iterator<char> beg(fr);
        istreambuf_iterator<char> end;
        content = string(beg, end);
    } catch (const exception &e) {
        cout<<"Load file failed: "<<e.what()<<endl;
    }

    return content;
}

static nlohmann::json
parse_json_file(const string &filename)
{
    nlohmann::json json;
    string content = load_filename(filename);
    if (content.empty()) {
        return json;
    }
    try {
    json = nlohmann::json::parse(content);
    } catch (const exception &e) {
        cout<<"Parse content failed: "<<e.what()<<endl;
    }
    return json;
}

int
main(int argc, char *argv[])
{
    if (argc != 4) {
        cout<<"Usage: "<<argv[0]<<" <shell> <mark software file> <avail software file>"<<endl;
        return -1;
    }

    nlohmann::json mark_json = parse_json_file(argv[2]);
    nlohmann::json avail_json = parse_json_file(argv[3]);
    if (mark_json.is_null() || avail_json.is_null()) {
        return -1;
    }

    nlohmann::json values;
    values["MARK_SOFTWARES"] = mark_json;
    values["AVAILABLE_SOFTWARES"] = avail_json["available_softwares"];
    vector<string> args = {values.dump(-1)};

    namespace dmr = dmcg::module::runner;
    dmr::TaskManager manager;
    boost::shared_ptr<dmr::Task> task = manager.Create(argv[1], args);
    if (!task) {
        cout<<"Create task failed"<<endl;
        return -1;
    }

    task->Finish.connect([task](
        const string &exception,
        const int &exit_code,
        const string &output,
        const string &error_output
    ){
        if (!exception.empty()) {
            cout<<"Exception: "<<exception<<endl;
            return;
        }
        cout<<"Exit code: "<<exit_code<<endl;
        if (!error_output.empty()) {
            cout<<"stderr: "<<error_output<<endl;
            return;
        }
        cout<<"stdout: "<<output<<endl;
    });
    task->Run(true);

    return 0;
}