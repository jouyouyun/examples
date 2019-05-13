#include "file.h"

#include <exception>
#include <boost/algorithm/string.hpp>

#include <iostream>

namespace jouyouyun
{
namespace network
{
namespace device
{
using namespace std;

KeyValueFile::KeyValueFile(const string &filepath,
                           const string &sep, ios::openmode mode) : line_sep(sep)
{
    try {
        this->core = unique_ptr<fstream>(new fstream);
        this->core->open(filepath, mode);
    } catch (exception &e) {
        throw string("Failed to open file:") + e.what();
        return;
    }
}

KeyValueFile::~KeyValueFile()
{
}

string KeyValueFile::LoadContent()
{
    string content;
    try {
        istreambuf_iterator<char> begin(*(this->core));
        istreambuf_iterator<char> end;
        content = string(begin, end);
    } catch (exception &e) {
        cout << "Failed to load file content:" << e.what() << endl;
    }
    return content;
}

string KeyValueFile::GetKey(const string &key)
{
    try {
        // must exec 'clear' before 'seekg'.
        // because of 'core' if reached the end of file, will set the eofbit, seekg will not work
        this->core->clear();
        this->core->seekg(ios::beg);
    } catch (exception &e) {
        cout << "Failed to seek file:" << e.what() << endl;
        return string("");
    }

    string result;
    string line;
    while (getline(*(this->core), line)) {
        vector<string> list = SplitString(line, this->line_sep);
        if (list.size() != 2) {
            continue;
        }
        boost::algorithm::trim(list[0]);
        if (list[0] != key) {
            continue;
        }
        result = list[1];
        break;
    }
    // TODO(jouyouyun): remove
    boost::algorithm::trim_left_if(result,
                                   boost::algorithm::is_any_of("\t"));
    return result;
}

vector<string> SplitString(const string &str, const string &sep)
{
    vector<string> items;
    if (str.empty()) {
        return items;
    }
    if (sep.empty()) {
        items.push_back(str);
        return items;
    }

    string::size_type pos = 0;
    pos = str.find(sep, 0);
    if (pos == string::npos) {
        items.push_back(str);
        return items;
    }    

    items.push_back(str.substr(0, pos));
    items.push_back(str.substr(pos + sep.size()));
    return items;
}
} // namespace device
} // namespace network
} // namesapce jouyouyun
