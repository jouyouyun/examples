#include "lsblk.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include "../runner/task.h"
#include "../runner/task_manager.h"

namespace jouyouyun {
namespace disk {
namespace lsblk {
using namespace std;

class LsblkPrivate {
    friend class Lsblk;
public:
    LsblkPrivate() {
        string outs;
        this->RunLsblk(outs);
        if (outs.empty()) {
            throw string("run lsblk failure");
        }

        this->UnmarshalOutput(outs);
    }
    ~LsblkPrivate() {
        this->infos.clear();
    }
private:
    nlohmann::json json_obj;
    vector<LsblkInfo> infos;

    void RunLsblk(string &outs);
    void UnmarshalOutput(const string &outs);
    bool IsRootDisk(nlohmann::json &children);
};

Lsblk::Lsblk() {
    try {
        this->core = unique_ptr<LsblkPrivate>(new LsblkPrivate());
    } catch (string what) {
        cout<<"Failed to init: "<<what<<endl;
    }
}

Lsblk::~Lsblk() {}

vector<LsblkInfo> Lsblk::List()
{
    return this->core->infos;
}

/**
 * lsblk output:
{
   "blockdevices": [
      {"name": "sda", "serial": "TF0500WE0GAV0V", "type": "disk", "size": "465.8G", "vendor": "ATA     ", "model": "HGST HTS725050A7", "mountpoint": null,
         "children": [
            {"name": "sda1", "serial": null, "type": "part", "size": "4G", "vendor": null, "model": null, "mountpoint": "/boot"},
            {"name": "sda2", "serial": null, "type": "part", "size": "4G", "vendor": null, "model": null, "mountpoint": "[SWAP]"},
            {"name": "sda3", "serial": null, "type": "part", "size": "1K", "vendor": null, "model": null, "mountpoint": null},
            {"name": "sda5", "serial": null, "type": "part", "size": "100G", "vendor": null, "model": null, "mountpoint": "/Data"},
            {"name": "sda6", "serial": null, "type": "part", "size": "60G", "vendor": null, "model": null, "mountpoint": "/"}
         ]
      }
   ]
}
 **/
void LsblkPrivate::UnmarshalOutput(const string &outs)
{
    this->infos.clear();
    try {
        json_obj = nlohmann::json::parse(outs);
        nlohmann::json devices = json_obj["blockdevices"];
        nlohmann::json::iterator iter = devices.begin();
        for (; iter != devices.end(); iter++) {
            nlohmann::json ty = (*iter)["type"];
            if (ty.is_null() || string(ty).compare("disk") != 0) {
                continue;
            }

            nlohmann::json name = (*iter)["name"];
            nlohmann::json serial = (*iter)["serial"];
            nlohmann::json size = (*iter)["size"];
            nlohmann::json vendor = (*iter)["vendor"];
            nlohmann::json model = (*iter)["model"];
            LsblkInfo info = {
                .name = name.is_null() ? "" : name,
                .serial = serial.is_null() ? "" : serial,
                .type = ty,
                .size = size.is_null() ? "" : size,
                .vendor = vendor.is_null() ? "":vendor,
                .model = model.is_null() ? "":model,
            };

            boost::algorithm::trim_right_if(info.model, boost::algorithm::is_any_of(" "));
            boost::algorithm::trim_right_if(info.vendor, boost::algorithm::is_any_of(" "));
            nlohmann::json children = (*iter)["children"];
            info.root = this->IsRootDisk(children);
            this->infos.push_back(info);
        }
    } catch (const exception &e) {
        cout<<"Failed to unmarshal: "<<e.what()<<endl;
    }
}

void LsblkPrivate::RunLsblk(string &outs)
{
    namespace dmr = dmcg::module::runner;
    dmr::TaskManager manager;
    boost::shared_ptr<dmr::Task> task;
    static vector<string> args = {
        "-c",
        "lsblk -J -no NAME,SERIAL,TYPE,SIZE,VENDOR,MODEL,MOUNTPOINT"
    };

    task = manager.Create("/bin/sh", args);
    task->Finish.connect([task, &outs](
                             const string &exception,
                             const int &exit_code,
                             const string &output,
                             const string &error_output
    ) {
        if (!exception.empty()) {
            cout<<"Failed to run lsblk: "<<exception<<endl;
            return;
        }
        if (!error_output.empty()) {
            cout<<"Failed to run lsblk: "<<exception<<endl;
            return;
        }
        outs = output;
    });
    task->Run(true);
}

bool LsblkPrivate::IsRootDisk(nlohmann::json &children)
{
    if (children.size() == 0) {
        return false;
    }

    nlohmann::json::iterator iter = children.begin();
    for (; iter != children.end(); iter++) {
        nlohmann::json mp = (*iter)["mountpoint"];
        if (mp.is_null()) {
            continue;
        }
        string mountpoint = mp;
        if (mountpoint.compare("/") == 0) {
            return true;
        }
    }

    return false;
}
} // namespace lsblk
} // namespace disk
} // namespace jouyouyun