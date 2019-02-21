#include <iostream>

#include "file.hpp"
#include "utils.hpp"

using namespace std;

void dump_vector(vector<string>& list)
{
    vector<string>::iterator iter = list.begin();
    for (; iter != list.end(); iter++) {
        cout<<"\t:"<<*iter<<endl;
    }
}

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        cout<<"Please input filepath"<<endl;
        return -1;
    }

    string filepath(argv[1]);
    File file(filepath, ios::in);
    // string contents = file.GetContents();
    // cout<<"Contents:\t"<<contents;

    string key("MemTotal");
    string value = file.GetKey(key);
    cout<<"MemTotal:"<<value<<"[END]"<<endl;

    vector<string> list;
    SplitString(string("name:Joy"), list, string(":"), 2);
    cout<<"List size(name:Joy):\t"<<list.size()<<endl;
    dump_vector(list);

    list.clear();
    SplitString(string("name:Joy:May:Toy"), list, string(":"), 2);
    cout<<"List size(name:Joy:May:Toy):\t"<<list.size()<<endl;
    dump_vector(list);

    list.clear();
    SplitString(string("name:Joy:May:Toy"), list, string(":"));
    cout<<"List size(name:Joy:May:Toy):\t"<<list.size()<<endl;
    dump_vector(list);

    string test1("   test   ");
    cout<<"After trim both:"<<TrimString(test1)<<"[END]"<<endl;

    test1 = "   test   ";
    cout<<"After trim left:"<<TrimString(test1, " ", TRIM_LEFT)<<"[END]"<<endl;

    test1 = "   test   ";
    cout<<"After trim right:"<<TrimString(test1, " ", TRIM_RIGHT)<<"[END]"<<endl;

    test1 = "\t   test   ";
    cout<<"After trim tab both:"<<TrimString(test1, "\t")<<"[END]"<<endl;

    test1 = "\t   test   ";
    cout<<"After trim tab right:"<<TrimString(test1, "\t", TRIM_RIGHT)<<"[END]"<<endl;

    return 0;
}
