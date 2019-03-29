#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;

// json data example:
// [
//     {"name": "xx", history: [
//                                 {"startup": timestamp, "shutdown": timestamp}
//                             ]
//     }
// ]
nlohmann::json generate()
{
    nlohmann::json obj = nlohmann::json::array();
    nlohmann::json soft;
    nlohmann::json his;

    soft["history"] = nlohmann::json::array();
    his["startup"] = 13762342382;
    his["shutdown"] = 13762343382;
    soft["history"].push_back(his);

    his["startup"] = 15762342382;
    his["shutdown"] = 15762543382;
    soft["history"].push_back(his);

    soft["name"] = "gedit";
    obj.push_back(soft);

    return obj;
}

nlohmann::json loadJsonFile(const string &filename)
{
    fstream fr;

    try {
        fr.open(filename);
    } catch (exception &e) {
        cout <<"Open failed failed: "<<e.what()<<endl;
        return nlohmann::json::array();
    }

    istreambuf_iterator<char> beg(fr);
    istreambuf_iterator<char> end;
    string data(beg, end);
    nlohmann::json obj = nlohmann::json::array();
    if (data.empty()) {
        return obj;
    }
    obj = nlohmann::json::parse(data);
    return obj;
}

void addShutdown(nlohmann::json &obj, const string &package, int64_t t)
{
    nlohmann::json::iterator it = obj.begin();
    for (; it != obj.end(); it++) {
        string v = it->value("name", "");
        if (v != package) {
            continue;
        }

        nlohmann::json &his = (*it)["history"];
        cout<<"Found "<<package<<", history: "<<his.size()<<endl;
        nlohmann::json::iterator tmp = his.end() - 1;
        (*tmp)["shutdown"] = t;
       break;
    }
}

int saveJson(nlohmann::json &obj, const string &filename)
{
    fstream fw;

    try {
        fw.open(filename, ios::out);
    } catch (exception &e) {
        cout <<"Open failed failed: "<<e.what()<<endl;
        return -1;
    }

    string data = obj.dump();
    cout<<"Will save data: "<<data<<endl;
    fw << data;
    fw.sync();
    fw.close();
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout<<"Usage: "<<argv[0]<<" <output file>"<<endl;
        return 0;
    }

    nlohmann::json obj = generate();
    int ret = saveJson(obj, string(argv[1]));
    if (ret != 0) {
        cout<<"Failed to save file"<<endl;
        return ret;
    }

    obj = loadJsonFile(string(argv[1]));
    if (obj.size() == 0) {
        cout<<"Failed to load file or file non-exists"<<endl;
        return -1;
    }

    addShutdown(obj, "gedit", 19458743242);
    ret = saveJson(obj, string(argv[1]));
    if (ret != 0) {
        cout<<"Failed to save file"<<endl;
        return ret;
    }

    obj.clear();
    cout<<"After clear: "<<obj.dump()<<", null: "<<obj.is_null()<<endl;
    nlohmann::json ttt;
    cout<<"null: "<<ttt.is_null()<<endl;

    return 0;
}
