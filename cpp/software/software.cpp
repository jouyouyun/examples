#include "software.h"

#include <vector>

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

// http://manpages.ubuntu.com/manpages/xenial/man8/apt-get.8.html --> --force-yes
static const string APT_OPTIONS = " --allow-downgrades --allow-remove-essential --allow-change-held-packages ";
// https://raphaelhertzog.com/2010/09/21/debian-conffile-configuration-file-managed-by-dpkg/
static const string DPKG_OPTIONS = " -o Dpkg::Options::=\"--force-confdef\" -o Dpkg::Options::=\"--force-confold\" ";

Software::Software()
{
}

Software::~Software()
{
}

SoftwareInfo *Software::Get(const string &name)
{
    if (name.empty()) {
        return NULL;
    }
    SoftwareInfo *info = new SoftwareInfo(name);
    return info;
}

SoftwareInfoList *Software::GetListIter()
{
    SoftwareInfoList *list = new SoftwareInfoList();
    return list;
}

vector<SoftwareInfo *> Software::GetList()
{
    vector<SoftwareInfo *> infos;
    SoftwareInfoList list;
    SoftwareInfo *info = list.Next();

    while (info) {
        infos.push_back(info);
        info = list.Next();
    }
    return infos;
}

void Software::FreeList(vector<SoftwareInfo *> &list)
{
    if (list.size() == 0) {
        return ;
    }

    vector<SoftwareInfo *>::iterator it = list.begin();
    for (; it != list.end(); it++) {
        delete *it;
    }
    list.clear();
}

Executor *Software::InstallPackage(const string &name)
{
    // NOTICE(jouyouyun): program will block if update grub/xkeyboard
    string cmd = "apt-get install -y" + APT_OPTIONS + DPKG_OPTIONS + name;
    Executor *exec = new Executor(cmd);
    exec->Start();
    return exec;
}

Executor *Software::RemovePackage(const string &name)
{
    string cmd = "apt-get purge -y" + APT_OPTIONS + name;
    Executor *exec = new Executor(cmd);
    exec->Start();
    return exec;
}
} // namespace software
} // namespace module
} // namespace dmcg
