#include "hostname.h"

#include <iostream>
#include <fstream>

namespace jouyouyun {
namespace example {
namespace unittest {
using namespace std;

class HostnamePrivate {
    friend class Hostname;

public:
    HostnamePrivate(const string &file): filepath(file) {}
    ~HostnamePrivate() {}

private:
    string filepath;
};

Hostname::Hostname(const string &file)  {
    this->core = std::unique_ptr<HostnamePrivate>(new HostnamePrivate(file));
}

Hostname::~Hostname() {}

string Hostname::Get()
{
    string name;
    try {
        fstream fr;
        fr.open(this->core->filepath, ios::in);
        istreambuf_iterator<char> begin(fr);
        istreambuf_iterator<char> end;
        name = string(begin, end);
    } catch (exception &e) {
        cout<<"Failed to get hostname: "<<e.what()<<endl;
    }

    return name;
}

int Hostname::Set(const string &name)
{
    try {
        fstream fw;
        fw.open(this->core->filepath, ios::out | ios::trunc);
        fw<<name;
        fw.close();
    } catch (exception &e) {
        cout<<"Failed to set hostname: "<<e.what()<<endl;
        return -1;
    }
    return 0;
}
} // namesapce unittest
} // namespace example
} // namespace jouyouyun