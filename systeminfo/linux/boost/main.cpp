#include <iostream>
#include <vector>

#include "systeminfo.hpp"

using namespace std;

namespace dsinfo = dmcg::module::systeminfo;

void DumpDMI(dsinfo::SystemInfo *sys_info)
{
    cout<<"Bios:"<<endl;
    cout<<"\tDate: "<<sys_info->GetBiosDate()<<endl;
    cout<<"\tVendor: "<<sys_info->GetBiosVendor()<<endl;
    cout<<"\tVersion: "<<sys_info->GetBiosVersion()<<endl;

    cout<<"Board:"<<endl;
    cout<<"\tName: "<<sys_info->GetBoardName()<<endl;
    cout<<"\tVendor: "<<sys_info->GetBoardVendor()<<endl;
    cout<<"\tVersion: "<<sys_info->GetBoardVersion()<<endl;
    cout<<"\tSerial: "<<sys_info->GetBoardSerial()<<endl;
    cout<<"\tAssetTag: "<<sys_info->GetBoardAssetTag()<<endl;

    cout<<"Product:"<<endl;
    cout<<"\tName: "<<sys_info->GetProductName()<<endl;
    cout<<"\tFamily: "<<sys_info->GetProductFamily()<<endl;
    cout<<"\tVersion: "<<sys_info->GetProductVersion()<<endl;
    cout<<"\tSerial: "<<sys_info->GetProductSerial()<<endl;
    cout<<"\tUUID: "<<sys_info->GetProductUUID()<<endl;
}

void DumpOS(dsinfo::SystemInfo *sys_info)
{
    cout<<"OS:"<<endl;
    cout<<"\tMachine ID: "<<sys_info->GetMachineID()<<endl;
    cout<<"\tName: "<<sys_info->GetOSName()<<endl;
    cout<<"\tVersion: "<<sys_info->GetOSVersion()<<endl;
    cout<<"\tType: "<<sys_info->GetOSType()<<endl;
    cout<<"\tCPU: "<<sys_info->GetCPU()<<endl;
    cout<<"\tMemory Capacity: "<<sys_info->GetMemoryCap()<<endl;
    cout<<"\tDisk Capacity: "<<sys_info->GetDiskCap()<<endl;
}

void DumpNetworkDevice(dsinfo::SystemInfo *sys_info)
{
    vector<dsinfo::NetDevInfo> devList = sys_info->GetNetworkDeviceList();
    cout<<"Network Device:"<<endl;
    if (devList.size() == 0) {
        cout<<"\tNo device found"<<endl;
    }
    vector<dsinfo::NetDevInfo>::iterator iter = devList.begin();
    for (; iter != devList.end(); iter++) {
        cout<<"\tIFace: "<<iter->iface<<", mac: "<<iter->macaddress<<endl;
        cout<<"\t\tIP: "<<iter->address<<endl;
    }
}

int main()
{
    dsinfo::SystemInfo *sys_info = new dsinfo::SystemInfo;

    DumpOS(sys_info);
    DumpDMI(sys_info);
    DumpNetworkDevice(sys_info);

    // cout<<"---------- Get again ----------"<<endl;
    // DumpOS(sys_info);
    // DumpDMI(sys_info);
    // DumpNetworkDevice(sys_info);

    delete sys_info;
    return 0;
}
