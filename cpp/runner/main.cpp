#include "task_manager.h"
#include "task.h"

#include <iostream>
#include <vector>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace dmcg::module::runner;

vector<string> MakeArgs(const string &package);

int main()
{
	vector<string> args;
	TaskManager manager;
	boost::shared_ptr<Task> task;

	args = MakeArgs("/home/wen");
	task = manager.Create("ls", args);
	task->Finish.connect([](const string &exception, 
				int exit_code,
				const string &output,
				const string &error_output){
				cout<<"Exception: "<<exception<<endl;
				cout<<"Exit code: "<<exit_code<<endl;
				cout<<"Stdout: "<<output<<endl;
				cout<<"Stderr: "<<error_output<<endl;
			});
	task->Run();
	return 0;
}

vector<string> MakeArgs(const string& package)
{
	vector<string> args;
	args.push_back("-l");
	args.push_back("-h");
	if (!package.empty()) {
		args.push_back(package);
	}
	return args;
}
