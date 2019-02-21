#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void dump(vector<string>& list)
{
    cout<<"Dump:"<<endl;
    vector<string>::iterator iter = list.begin();
    for (; iter != list.end(); iter++) {
        cout<<"\t"<<*iter<<endl;
    }
}

int main()
{
    vector<string> list;

    list.push_back("enp0s25");
    list.push_back("lo");
    list.push_back("wlp1s23");
    list.push_back("docker0");
    list.push_back("docker1");
    list.push_back("docker2");
    list.push_back("docker3");
    dump(list);

    vector<string>::iterator iter = find(list.begin(), list.end(), "lo");
    if (iter != list.end()) {
        cout<<"Found: "<<*iter<<endl;
        cout<<"Index:"<<iter-list.begin()<<endl;
        list.erase(iter);
    } else {
        cout<<"Not found: lo"<<endl;
    }
    dump(list);

    iter = list.begin();
    int count = 0;
    for (;iter != list.end();iter++) {
        cout<<"Iterator: "<<*iter<<", count: "<<count<<endl;
        if ((iter-list.begin()+count) % 2 == 0) {
            // after erased, the iter move to next
            list.erase(iter);
            cout<<"Next: "<<*iter<<endl;
            iter--;
            count++;
        }
    }
    dump(list);
    return 0;
}
