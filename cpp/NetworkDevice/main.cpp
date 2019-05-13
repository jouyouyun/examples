#include "network_device.h"

#include <iostream>

using namespace std;
using namespace jouyouyun::network::device;

int 
main()
{
    NetworkDevice dev;
    vector<DeviceInfo> infos = dev.List();
    vector<DeviceInfo>::iterator it = infos.begin();
    for (; it != infos.end(); it++) {
        cout<<"Name: "<<it->name<<endl;
        cout<<"\tInterface: "<<it->interface<<endl;
        cout<<"\tMacaddress: "<<it->macaddress<<endl;
        cout<<"\tIP: "<<it->ip<<endl;
    }

    return 0;
}