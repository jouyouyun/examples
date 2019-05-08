#include <iostream>
#include "lsblk.h"

using namespace std;
using namespace jouyouyun::disk::lsblk;

int
main(int argc, char *argv[])
{
    Lsblk obj;
    vector<LsblkInfo> infos = obj.List();
    if (infos.size() == 0) {
        cout<<"No device found"<<endl;
        return 0;
    }

    vector<LsblkInfo>::iterator iter = infos.begin();
    for (; iter != infos.end(); iter++) {
        cout<<iter->DisplayName()<<endl;
        cout<<"\tName: "<<iter->name<<endl;
        cout<<"\tmodel: "<<iter->model<<"-"<<iter->serial<<"-"<<iter->vendor<<endl;
        cout<<"\ttype: "<<iter->type<<endl;
        cout<<"\tsize: "<<iter->size<<endl;
        cout<<"\troot: "<<iter->root<<endl;
    }

    return 0;
}