#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <nlohmann/json.hpp>

#include "software_history.h"
#include <iostream>
//#include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace software
{
using namespace std;

#define SOFT_HIST_FILE "/var/lib/dmcg/software_history.json"

class SoftHistoryPrivate
{
    friend SoftwareHistory;

public:
    SoftHistoryPrivate();
    ~SoftHistoryPrivate();
private:
    void Add(const string &pkg, bool startup);
    int Save();
    bool EnsureDirExist(const string &filename);
    int64_t GetTimestamp();

    nlohmann::json obj; // is null
    bool has_trunc;
};

SoftwareHistory::SoftwareHistory()
{
    d = unique_ptr<SoftHistoryPrivate>(new SoftHistoryPrivate);
    cout << "Init soft hist: " << &d << endl;
}

SoftwareHistory::~SoftwareHistory()
{
}

void SoftwareHistory::SaveStartup(const string &pkg)
{
    cout << "Obj size: " << d->obj.size() << endl;
    d->Add(pkg, true);
    d->Save();
}

void SoftwareHistory::SaveShutdown(const string &pkg)
{
    d->Add(pkg, false);
    d->Save();
}

string SoftwareHistory::Dump()
{
    if (d->obj.is_null()) {
        return "";
    }

    // TODO(jouyouyun): add locker
    string data = d->obj.dump();
    d->obj.clear();
    d->has_trunc = true;

    return data;
}

SoftHistoryPrivate::SoftHistoryPrivate()
{
    fstream fr;
    try {
        fr.open(string(SOFT_HIST_FILE), ios::in);
    } catch (exception &e) {
        //LOG_ERROR << "open software history file failed:" << e.what();
        cout << "open software history file failed:" << e.what() << endl;
        return;
    }

    istreambuf_iterator<char> beg(fr);
    istreambuf_iterator<char> end;
    string content(beg, end);
    fr.close();

    cout << "Load file: " << content << ", empty: " << content.empty() << endl;
    if (content.empty()) {
        obj = nlohmann::json::array();
        return;
    }

    try {
        obj = nlohmann::json::parse(content);
    } catch (exception &e) {
        //LOG_ERROR << "json parse failed:" << e.what();
        cout << "json parse failed:" << e.what() << endl;
    }
}

SoftHistoryPrivate::~SoftHistoryPrivate()
{}

void SoftHistoryPrivate::Add(const string &pkg, bool startup)
{
    if (obj.is_null()) {
        cout << "Object null" << endl;
        return ;
    }

    cout << "Add: " << pkg << ", size: " << obj.size() << endl;
    bool found = false;
    nlohmann::json::iterator it = obj.begin();
    if (obj.size() == 0) {
        goto out;
    }

    for (; it != obj.end(); it++) {
        string v = it->value("name", "");
        if (v != pkg) {
            continue;
        }

        found = true;
        nlohmann::json &his = (*it)["history"];
        if (startup) {
            nlohmann::json tmp = nlohmann::json::object();
            tmp["startup"] = GetTimestamp();
            his.push_back(tmp);
            break;
        }

        nlohmann::json::iterator last = his.end() - 1;
        (*last)["shutdown"] = GetTimestamp();
        break;
    }

    if (found || !startup) {
        return;
    }

out:
    nlohmann::json soft = nlohmann::json::object();
    nlohmann::json tmp = nlohmann::json::object();
    tmp["startup"] = GetTimestamp();
    soft["history"] = nlohmann::json::array();
    soft["history"].push_back(tmp);
    soft["name"] = pkg;
    obj.push_back(soft);
}

int SoftHistoryPrivate::Save()
{
    if (obj.is_null()) {
        return 0;
    }

    if (!EnsureDirExist(SOFT_HIST_FILE)) {
        cout << "Ensure dir exist failed" << endl;
        return -1;
    }

    fstream fw;
    ios_base::openmode m = ios::out;
    if (has_trunc) {
        m |= ios::trunc;
        has_trunc = false;
    }
    try {
        fw.open(string(SOFT_HIST_FILE), m);
    } catch (exception &e) {
        cout << "open software history file failed:" << e.what();
        return -1;
    }

    string data = obj.dump();
    fw.write(data.c_str(), data.size());
    fw.close();

    return 0;
}

bool SoftHistoryPrivate::EnsureDirExist(const string &filename)
{
    namespace bf = boost::filesystem;
    bf::path file(filename);
    bf::path dir = file.parent_path();

    if (bf::exists(dir)) {
        return true;
    }

    return bf::create_directories(dir);
}

int64_t SoftHistoryPrivate::GetTimestamp()
{
    namespace bpt = boost::posix_time;

    bpt::ptime epoch(boost::gregorian::date(1970,
                                            boost::gregorian::Jan,
                                            1));
    cout << "Date start: " << epoch.base_time::date() << endl;
    bpt::time_duration dur = bpt::second_clock::universal_time() - epoch;
    return dur.total_seconds();
}
} // namespace software
} // namespace module
} // namespace dmcg
