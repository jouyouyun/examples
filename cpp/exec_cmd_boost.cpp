#include <iostream>
#include <vector>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace bp = boost::process;

void read_lsblk_output(vector<string>& lines);

// disk type: [disk, part, loop, rom]
int main()
{
    vector<string> lines;
    read_lsblk_output(lines);
    if (lines.size() == 0) {
        return -1;
    }
    // delete 'NAME   MAJ:MIN RM         SIZE RO TYPE MOUNTPOINT'
    lines.erase(lines.begin());

    cout<<"Outputs:"<<endl;
    vector<string>::iterator iter = lines.begin();
    for (; iter != lines.end(); iter++) {
        cout<<*iter<<endl;
    }
    return 0;
}

void read_lsblk_output(vector<string>& lines)
{
    string line;
    bp::ipstream is; // reading pipe-stream
    bp::child c(bp::search_path("lsblk"), "-ab", bp::std_out>is);

    while(c.running() && getline(is, line)&& !line.empty()) {
        lines.push_back(line);
    }
    c.wait();
}
