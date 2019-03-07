#include "netlink_monitor.h"
#include "software_guard.h"

#include <iostream>

using namespace std;

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <blacklist file>" << endl;
        return -1;
    }

    namespace nsoft = dmcg::module::software;
    nsoft::SoftwareGuard guard(argv[1]);
    guard.ReloadBlacklist(argv[1]);
    guard.Loop();
    return 0;
}
