#include "blacklist.h"

#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include <iostream>
// #include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

class BlacklistPrivate
{
    friend class Blacklist;

public:
    BlacklistPrivate();
    BlacklistPrivate(const string &filename);
    ~BlacklistPrivate();

private:
    vector<string> blacklist;

    void LoadFile(vector<string> &list, const string &filename,
                  bool append);
};

Blacklist::Blacklist()
{
    d = unique_ptr<BlacklistPrivate>(new BlacklistPrivate);
}

Blacklist::Blacklist(const string &filename)
{
    d = unique_ptr<BlacklistPrivate>(new BlacklistPrivate(filename));
}

Blacklist::~Blacklist()
{}

void Blacklist::SetBlacklist(const string &filename, bool append)
{
    d->LoadFile(d->blacklist, filename, append);
}

bool Blacklist::IsInList(const string &name)
{
    if (d->blacklist.size() == 0) {
        return false;
    }

    vector<string>::iterator it = d->blacklist.begin();
    for (; it != d->blacklist.end(); it++) {
        // Why '*it == name' not working?
        if (strcmp(it->c_str(), name.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

BlacklistPrivate::BlacklistPrivate()
{
    blacklist.clear();
}

BlacklistPrivate::BlacklistPrivate(const string &filename)
{
    LoadFile(blacklist, filename, false);
}

void BlacklistPrivate::LoadFile(vector<string> &list, const string &filename,
                                bool append)
{
    fstream fr;
    string line;

    if (!append) {
        list.clear();
    }

    try {
        fr.open(filename, ios::in);
    } catch (exception &e) {
        // LOG_ERROR << "open file failed: " << e.what();
        cout << "open file failed: " << e.what() << endl;
        return;
    }

    try {
        while (getline(fr, line)) {
            if (line.empty()) {
                continue;
            }
            boost::algorithm::trim_right_if(line,
                                            boost::algorithm::is_any_of("\n"));
            cout << "Push to blacklist: " << line << endl;
            list.push_back(line);
        }
    } catch (exception &e) {
        fr.close();
        // LOG_WARN << "read file failed: " << e.what();
        cout << "read file failed: " << e.what() << endl;
        return;
    }
    fr.close();
}

BlacklistPrivate::~BlacklistPrivate()
{
    blacklist.clear();
}
} // namespace software
} // namespace module
} // namespace dmcg
