#include "utils.hpp"

void SplitString(const string& str, vector<string>& list,
                 const string& delim, const int num)
{
    if (str.empty()) {
        return;
    }

    if (delim.empty()) {
        list.push_back(str);
        return;
    }

    string::size_type pos1 = 0, pos2 = 0;
    int count = 0;

    pos2 = str.find(delim, 0);
    while (string::npos != pos2) {
        if (num - count == 1) {
            break;
        }
        list.push_back(str.substr(pos1, pos2-pos1));
        count++;
        pos1 = pos2 + delim.size();
        pos2 = str.find(delim, pos1);
    }

    if ((num < 0 || num - count > 0) && string::npos != str.length()) {
        list.push_back(str.substr(pos1));
    }
}

string& TrimString(string& str, const string delim, const int mode)
{
    if (str.empty() || delim.empty()) {
        return str;
    }

    if ((mode & TRIM_LEFT) == TRIM_LEFT) {
        str.erase(0, str.find_first_not_of(delim));
    }
    if ((mode & TRIM_RIGHT) == TRIM_RIGHT) {
        str.erase(str.find_last_not_of(delim)+1);
    }
    return str;
}
