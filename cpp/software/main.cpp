#include "software.h"
#include "executor.h"
#include <iostream>

#include <boost/thread.hpp>

using namespace std;

void TestSoftwareInfo();
void TestExecutor();
void TestSoftwareExecutor();

int main()
{
    TestSoftwareInfo();
    // TestExecutor();
    TestSoftwareExecutor();

    return 0;
}

void TestSoftwareExecutor()
{
    namespace nsoft = dmcg::module::software;
    nsoft::Software soft;

    nsoft::Executor *exec;
    string package = "google-chrome-stable";
    nsoft::SoftwareInfo *info = soft.Get(package);
    if (info->name.empty()) {
        cout << "No " << package << " found, install" << endl;
        exec = soft.InstallPackage(package);
    } else {
        cout << "Found: " << info->name << " --- " << info->version << endl;
        delete info;
        cout << "Start remove:" << endl;
        exec = soft.RemovePackage(package);
    }

    while (exec->Running()) {
        // wait for finished
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }

    cout << "Result: " << exec->cmdline << ", status: " << exec->status << ", error: " << exec->error << endl;
    delete exec;
}

void TestSoftwareInfo()
{
    namespace nsoft = dmcg::module::software;
    nsoft::Software soft;
    int count = 1;

    cout << "Query list for top10" << endl;
    vector<unique_ptr<nsoft::SoftwareInfo>> infos = soft.GetList();
    vector<unique_ptr<nsoft::SoftwareInfo>>::iterator it = infos.begin();
    for (; it != infos.end(); it++) {
        if (count > 10) {
            break;
        }
        count++;
        cout << "\tPackage: " << (*it)->name << ", Version: " << (*it)->version << "Architecture" << (*it)->architecture << endl;
    }

    cout << "Query for 'util-linux'" << endl;
    nsoft::SoftwareInfo *info = soft.Get("util-linux");
    cout << "\tPackage: " << info->name << ", Version:" << info->version << endl;
    delete info;

    cout << "Query list iter for top10" << endl;
    nsoft::SoftwareInfoList *list = soft.GetListIter();
    info = list->Next();
    count = 1;
    while (info) {
        if (count > 10) {
            delete info;
            break;
        }
        count++;
        cout << "\tPackage: " << info->name << ", Version:" << info->version << endl;
        delete info;
        info = list->Next();
    }
    delete list;
}

void TestExecutor()
{
    namespace nsoft = dmcg::module::software;
    nsoft::Executor *exec = new nsoft::Executor("sleep 10 && echo 'Hello' >> /tmp/soft.log");
    exec->Start();
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    cout << "'" + exec->cmdline + "'" << " running: " << exec->Running() << ", status: " << exec->status << endl;
    while (exec->Running()) {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    cout << "\t[Done] running: " << exec->Running() << ", status: " << exec->status << endl;
}
