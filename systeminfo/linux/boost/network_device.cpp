#include "network_device.hpp"
#include "file.hpp"

#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            #define NETWORK_DEVICE_SYSFS_DIR "/sys/class/net/"

            NetworkDevice::NetworkDevice()
            {
                filter_names.push_back("lo");
                filter_devtype_list.push_back("bridge");
            }

            NetworkDevice::~NetworkDevice()
            {
                filter_names.clear();
                filter_devtype_list.clear();
            }

            vector<NetDevInfo> NetworkDevice::GetNetworkDeviceList()
            {
                vector<NetDevInfo> devList;
                vector<string> ifaces;
                ScanDir(NETWORK_DEVICE_SYSFS_DIR, ifaces);
                if (ifaces.size() == 0) {
                    return devList;
                }

                FilterByName(ifaces);
                FilterByDevType(ifaces);
                if (ifaces.size() == 0) {
                    return devList;
                }

                vector<string>::iterator iter = ifaces.begin();
                for (; iter != ifaces.end(); iter++) {
                    NetDevInfo info;
                    GetNetDevInfo(*iter, info);
                    devList.push_back(info);
                }
                return devList;
            }

            // filter iface DEVTYPE ['bridge']
            void NetworkDevice::FilterByDevType(vector<string>& ifaces)
            {
                if (ifaces.size() == 0) {
                    return;
                }
                vector<string>::iterator beg = filter_devtype_list.begin();
                vector<string>::iterator end = filter_devtype_list.end();
                vector<string>::iterator iter = ifaces.begin();
                while (iter != ifaces.end()) {
                    string filepath = NETWORK_DEVICE_SYSFS_DIR+*iter+"/uevent";
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
                    iter = ifaces.erase(iter);
                }
            }

            // filter iface named ['lo']
            void NetworkDevice::FilterByName(vector<string> &ifaces)
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

            void NetworkDevice::GetNetDevInfo(const string &name, NetDevInfo &info)
            {
                namespace al = boost::algorithm;
                info.iface = name;
                FilePrivate f(NETWORK_DEVICE_SYSFS_DIR+name+"/address");
                string addr = f.LoadContent();
                al::trim_right_if(addr, al::is_any_of("\n"));
                info.macaddress = addr;
            }

            void NetworkDevice::ScanDir(const string& filepath, vector<string>& ifaces)
            {
                namespace fs = boost::filesystem;
                if (!fs::exists(filepath)) {
                    return;
                }

                fs::recursive_directory_iterator iter(filepath);
                fs::recursive_directory_iterator end_iter;
                for(; iter != end_iter; iter++) {
                    try {
                        if (!fs::is_directory(*iter)) {
                            continue;
                        }
                        ifaces.push_back(fs::basename(iter->path().string()));
                    } catch (exception& e) {
                        cout<<"check file is directory failed:"<<e.what();
                    }
                }
                return;
            }
        }
    }
}
