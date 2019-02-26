#ifndef __MODULE_SYSTEMINFO_NETWORK_DEVICE_H__
#define __MODULE_SYSTEMINFO_NETWORK_DEVICE_H__

#include <ios>
#include <vector>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            using namespace std;

            typedef struct _NetworkDeviceInfo {
                string iface;
                string macaddress;
                string address;
            } NetDevInfo;

            class NetworkDevice {
            public:
                NetworkDevice();
                ~NetworkDevice();
                vector<NetDevInfo> GetNetworkDeviceList();
            private:
                vector<string> filter_names;
                vector<string> filter_devtype_list;

                void GetNetDevInfo(const string& name, NetDevInfo& info);
                void ScanDir(const string& filepath, vector<string>& ifaces);
                void FilterByName(vector<string>& ifaces);
                void FilterByDevType(vector<string>& ifaces);
                void GetIfaceAddress(const string& name, string& addr);
            };
        } // namespace systeminfo
    } // namespace module
} // namespace dmcg

#endif
