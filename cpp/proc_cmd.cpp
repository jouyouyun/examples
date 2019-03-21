#include <iostream>
#include <fstream>
#include <vector>
#include <boost/range/as_array.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

string GetContent(const string & filepath)
{
    fstream fr;
    string content;

    fr.open(filepath, ios::in);
    istreambuf_iterator<char> beg(fr);
    istreambuf_iterator<char> end;
    content = string(beg, end);
    return content;
}

void DumpContent(const string & content)
{
    int i = 0;
    int length = content.length();

    for (; i < length; i++) {
        cout<<"<"<<(int)(content[i] - 0)<<">"<<endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout<<"Usage: "<<argv[0]<<" <pid>"<<endl;
        return -1;
    }

    string filepath = "/proc/" + string(argv[1]) + "/cmdline";
    string content = GetContent(filepath);
    DumpContent(content);

    vector<string> items;
    namespace ba = boost::algorithm;
    // 拆分含有结束符('\0')的字符串序列
    ba::split(items, content, ba::is_any_of(boost::as_array("\0")));
    vector<string>::iterator it = items.begin();
    for (; it != items.end(); it++) {
        cout<<"Item: "<<*it<<endl;
    }

    return 0;
}
