#include <iostream>
#include <fstream>

#include "file.hpp"
#include "utils.hpp"

using namespace std;

File::File(const string& filepath, ios::openmode mode)
{
    fs.open(filepath, mode);
    if (!fs.good()) {
        throw "file open failure";
    }
}

File::~File()
{
    cout<<"[DEBUG] Destroy!"<<endl;
    if (!fs.good()) {
        return;
    }
    fs.close();
}

string File::GetContents()
{
    string contents ((istreambuf_iterator<char>(fs)),
                     istreambuf_iterator<char>());
    return contents;
}

string File::GetKey(const string& key, const string delim,
                 const int num)
{
    string value;
    fs.seekp(ios::beg);

    string line;
    while(getline(fs, line)) {
        vector<string> list;
        SplitString(line, list, delim, num);
        if (list.size() != (unsigned long)num) {
            continue;
        }
        if (list[0] != key) {
            continue;
        }
        value = list[1];
        break;
    }
    value = TrimString(value);
    return value;
}
