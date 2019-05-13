#include "network_device.h"
#include "file.h"

#include "../runner/task.h"
#include "../runner/task_manager.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace jouyouyun {
namespace network {
namespace device {
using namespace std;

class NetDevPrivate {
    friend class NetworkDevice;
public:
    NetDevPrivate(const string &sysfs, const string &virt):
        sysfs_dir(sysfs),
        virtual_dir(virt)
    {
        this->ScanInterfaceDir();
        vector<string>::iterator it = this->iface_list.begin();
        for (; it != this->iface_list.end(); it++) {
            if (this->IsVirtualDevice(*it)) {
                continue;
            }

            DeviceInfo info;
            this->MakeDeviceInfo(info, *it);
            this->infos.push_back(info);
        }
    }

    ~NetDevPrivate()
    {}

private:
    string sysfs_dir;
    string virtual_dir;
    vector<DeviceInfo> infos;
    vector<string> iface_list;

    void MakeDeviceInfo(DeviceInfo &info, const string &iface);
    string GetName(const string &iface);
    string GetNameByPCI(const string &iface);
    string GetNameByUSB(const string &iface);
    string GetIP(const string &iface);
    string GetMacaddress(const string &iface);
    void ScanInterfaceDir();
    bool IsVirtualDevice(const string &iface);
    void RunCommand(const string &cmd, int &ret, string &output);
    string GenUSBProduct(const string &product);
};

NetworkDevice::NetworkDevice(const string &sysfs_dir, const string &virtual_dir)
{
    this->core = unique_ptr<NetDevPrivate>(new NetDevPrivate(sysfs_dir,
                                           virtual_dir));
}

NetworkDevice::~NetworkDevice()
{}

vector<DeviceInfo> NetworkDevice::List()
{
    return this->core->infos;
}

void NetDevPrivate::MakeDeviceInfo(DeviceInfo &info, const string &iface)
{
    info.interface = iface;
    info.ip = this->GetIP(iface);
    info.macaddress = this->GetMacaddress(iface);
    info.name = this->GetName(iface);
}

string NetDevPrivate::GetName(const string &iface)
{
    const string filename = this->sysfs_dir + "/" + iface + "/device/uevent";
    try {
        KeyValueFile f(filename);
        string pci_slot = f.GetKey("PCI_SLOT_NAME");
        if (!pci_slot.empty()) {
            return this->GetNameByPCI(pci_slot);
        }

        string product = f.GetKey("PRODUCT");
        if (!product.empty()) {
            return this->GetNameByUSB(this->GenUSBProduct(product));
        }
    } catch (const string what) {
        cout<<"Failed to get name: "<<what<<endl;
    }
    return "";
}

string NetDevPrivate::GetNameByPCI(const string &pci_slot)
{
    int ret;
    string outs;

    this->RunCommand("lspci -s " + pci_slot, ret, outs);
    if (ret != 0) {
        cout<<"Failed to run lspci: "<<outs<<endl;
        return "";
    }

    vector<string> list1 = SplitString(outs, " (rev ");
    if (list1.size() != 2) {
        cout<<"Invalid lspci output: "<<outs<<endl;
        return "";
    }
    vector<string> list2 = SplitString(list1[0], ": ");
    if (list2.size() != 2) {
        cout<<"Invalid lspci output: "<<outs<<endl;
        return "";
    }
    return list2[1];
}

string NetDevPrivate::GetNameByUSB(const string &product)
{
    int ret;
    string outs;

    this->RunCommand("lsusb -d " + product, ret, outs);
    if (ret != 0) {
        cout<<"Failed to run lsusb: "<<outs<<endl;
        return "";
    }
    namespace ba = boost::algorithm;
    ba::trim_right_if(outs, ba::is_any_of("\n"));
    vector<string> items = SplitString(outs, product + " ");
    if (items.size() != 2) {
        cout<<"Invalid lsusb output: "<<outs<<endl;
        return "";
    }
    return items[1];
}

string NetDevPrivate::GetIP(const string &iface)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        cout<<"Failed to open socket: "<<strerror(errno)<<endl;
        return "";
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    return string(inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
}

string NetDevPrivate::GetMacaddress(const string &iface)
{
    KeyValueFile fr(this->sysfs_dir + "/" + iface + "/address");
    string mac = fr.LoadContent();
    boost::algorithm::trim_right_if(mac, boost::algorithm::is_any_of("\n"));
    return mac;
}

void NetDevPrivate::ScanInterfaceDir()
{
    namespace bfs = boost::filesystem;
    try {
        bfs::path p(this->sysfs_dir);
        if (!bfs::exists(p)) {
            return ;
        }

        bfs::directory_iterator it(p);
        bfs::directory_iterator end;
        for (; it != end; it++) {
            if (!bfs::is_directory(*it)) {
                continue;
            }
            this->iface_list.push_back(it->path().filename().string());
        }
    } catch (exception &e) {
        cout<<"Failed to scan dir: "<< e.what()<<endl;
    }
}

bool NetDevPrivate::IsVirtualDevice(const string &iface)
{
    boost::filesystem::path p(this->virtual_dir + "/" + iface);
    return boost::filesystem::exists(p);
}

void NetDevPrivate::RunCommand(const string &cmd, int &ret, string &output)
{
    namespace dmr = dmcg::module::runner;
    dmr::TaskManager manager;
    boost::shared_ptr<dmr::Task> task;
    vector<string> args = {"-c", cmd};

    task = manager.Create("/bin/sh", args);
    task->Finish.connect([task, &ret, &output](
                             const string &exception,
                             const int &exit_code,
                             const string &outs,
                             const string &err_outs
    ) {
        if (!exception.empty()) {
            ret = -1;
            output = exception;
            return;
        }

        if (!err_outs.empty()) {
            ret = exit_code;
            output = err_outs;
        } else {
            ret = 0;
            output = outs;
        }
    });
    task->Run(true);
}

string NetDevPrivate::GenUSBProduct(const string &product)
{
    string name;
    vector<string> items;
    namespace ba = boost::algorithm;

    ba::split(items, product, ba::is_any_of("/"));
    vector<string>::iterator it = items.begin();
    // items size must be equal 3
    for (; it != (items.end() - 1); it++) {
        size_t len = it->size();
        string tmp = *it;
        if (len < 4) {
            for (size_t i = len; i < 4; i++) {
                tmp = "0" + tmp;
            }
        }
        if (it == items.begin()) {
            name = tmp;
        } else {
            name += ":" + tmp;
        }
    }

    return name;
}
} // namespace device
} // namespace network
} // namespace jouyouyun