#include <pstreams/pstream.h>
#include <string>
#include <vector>
#include <iostream>

#include <boost/algorithm/string.hpp>

using namespace std;

void parse_lsblk_line(const string& line);
void read_lsblk_outputs(vector<string>& lines);

int main()
{
    vector<string> lines;

    read_lsblk_outputs(lines);
    if (lines.size() == 0){
        return -1;
    }
    // delete 'NAME   MAJ:MIN RM         SIZE RO TYPE MOUNTPOINT'
    lines.erase(lines.begin());
    cout<<"Content:"<<endl;
    vector<string>::iterator iter = lines.begin();
    for (; iter != lines.end(); iter++) {
        cout<<"Line: "<<*iter<<endl;
        parse_lsblk_line(*iter);
    }
    return 0;
}

void read_lsblk_outputs(vector<string>& lines)
{
    // run a process and create a streambuf that reads its stdout and stderr
    redi::ipstream proc("lsblk -blp", redi::pstreams::pstdout | redi::pstreams::pstderr);
    string line;
    // read child's stdout
    while (getline(proc.out(), line) && !line.empty()) {
        lines.push_back(line);
    }
    // read child's stderr
    while (getline(proc.err(), line)) {
        cout << "stderr: " << line << '\n';
    }
}

void parse_lsblk_line(const string& line)
{
    namespace al = boost::algorithm;
    vector<string> items;
    al::split(items, line, al::is_any_of(" "));
    if (items.size() == 0) {
        return ;
    }

    vector<string>::iterator iter = items.begin();
    while(iter != items.end()) {
        if (!iter->empty()) {
            iter++;
            continue;
        }
        iter = items.erase(iter);
    }
}
