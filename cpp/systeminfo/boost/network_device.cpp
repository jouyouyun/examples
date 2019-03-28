#include "network_device.h"
#include "file.h"

#include <ios>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <iostream>
//#include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace systeminfo
{
using namespace std;

#define NETWORK_DEVICE_SYSFS_DIR "/sys/class/net/"

class NetworkDevicePrivate
{
    friend class NetworkDevice;

public:
    NetworkDevicePrivate();
    ~NetworkDevicePrivate();

private:
    std::vector<std::string> filter_names;
    std::vector<std::string> filter_devtype_list;

    void GetAddress(const string &iface_name);
    void GetNetDevInfo(const std::string &name, NetDevInfo &info);
    void ScanDir(const std::string &filepath,
                 std::vector<std::string> &ifaces);
    void FilterByName(std::vector<std::string> &ifaces);
    void FilterByDevType(std::vector<std::string> &ifaces);
    void GetIfaceAddress(const string &name, string &addr);
};

NetworkDevice::NetworkDevice()
{
    d = unique_ptr<NetworkDevicePrivate>(new NetworkDevicePrivate);
}

NetworkDevice::~NetworkDevice()
{
}

vector<NetDevInfo> NetworkDevice::GetNetworkDeviceList()
{
    vector<NetDevInfo> devList;
    vector<string> ifaces;
    d->ScanDir(NETWORK_DEVICE_SYSFS_DIR, ifaces);
    if (ifaces.size() == 0) {
        return devList;
    }

    d->FilterByName(ifaces);
    d->FilterByDevType(ifaces);
    if (ifaces.size() == 0) {
        return devList;
    }

    vector<string>::iterator iter = ifaces.begin();
    for (; iter != ifaces.end(); iter++) {
        NetDevInfo info;
        d->GetNetDevInfo(*iter, info);
        devList.push_back(info);
    }

    return devList;
}

NetworkDevicePrivate::NetworkDevicePrivate()
{
    filter_names.push_back("lo");
    filter_devtype_list.push_back("bridge");
}

NetworkDevicePrivate::~NetworkDevicePrivate()
{
    filter_names.clear();
    filter_devtype_list.clear();
}

// filter iface DEVTYPE ['bridge']
void NetworkDevicePrivate::FilterByDevType(vector<string> &ifaces)
{
    if (ifaces.size() == 0) {
        return;
    }
    vector<string>::iterator beg = filter_devtype_list.begin();
    vector<string>::iterator end = filter_devtype_list.end();
    vector<string>::iterator iter = ifaces.begin();
    while (iter != ifaces.end()) {
        string filepath = NETWORK_DEVICE_SYSFS_DIR + *iter + "/uevent";
        FilePrivate *fp = new FilePrivate(filepath);
        string devtype = fp->GetKey("DEVTYPE", "=");
        delete fp;
        fp = NULL;
        if (devtype.empty()) {
            iter++;
            continue;
        }

        if (find(beg, end, devtype) == end) {
            // not found
            iter++;
            continue;
        }
        // after erased, iter move to next item
        ifaces.erase(iter);
    }
}

// filter iface named ['lo']
void NetworkDevicePrivate::FilterByName(vector<string> &ifaces)
{
    if (ifaces.size() == 0) {
        return;
    }
    vector<string>::iterator iter = filter_names.begin();
    for (; iter != filter_names.end(); iter++) {
        vector<string>::iterator it = find(ifaces.begin(), ifaces.end(),
                                           *iter);
        if (it == ifaces.end()) {
            // not found
            continue;
        }
        ifaces.erase(it);
    }
}

void NetworkDevicePrivate::GetNetDevInfo(const string &name, NetDevInfo &info)
{
    namespace al = boost::algorithm;
    info.iface = name;
    FilePrivate f(NETWORK_DEVICE_SYSFS_DIR + name + "/address");
    string mac = f.LoadContent();
    al::trim_right_if(mac, al::is_any_of("\n"));
    info.macaddress = mac;
    GetIfaceAddress(info.iface, info.address);
}

void NetworkDevicePrivate::ScanDir(const string &filepath, vector<string> &ifaces)
{
    namespace fs = boost::filesystem;
    if (!fs::exists(filepath)) {
        return;
    }

    fs::recursive_directory_iterator iter(filepath);
    fs::recursive_directory_iterator end_iter;
    for (; iter != end_iter; iter++) {
        try {
            if (!fs::is_directory(*iter)) {
                continue;
            }
            ifaces.push_back(fs::basename(iter->path().string()));
        } catch (exception &e) {
            //LOG_WARN << "check file is directory failed:" << e.what();
            cout << "check file is directory failed:" << e.what()<<endl;
        }
    }
    return;
}

void NetworkDevicePrivate::GetIfaceAddress(const string &name,
        string &addr)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        //LOG_WARN << "open socket failed";
        cout << "open socket failed"<<endl;
        return;
    }

    // must init ifr
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    char *c_addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    addr = string(c_addr);
}
}
}
}
