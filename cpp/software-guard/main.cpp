#include "netlink_monitor.h"
#include "software_guard.h"

#include <iostream>

using namespace std;

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <whitelist file> <blacklist file>" << endl;
        return -1;
    }

    namespace nsoft = dmcg::module::software;
    nsoft::SoftwareGuard guard(argv[1], argv[2]);
    guard.Kill.connect([](const string & package) {
        cout << "Recieved Kill: " << package << endl;
    });
    guard.Loop();
    return 0;
}
