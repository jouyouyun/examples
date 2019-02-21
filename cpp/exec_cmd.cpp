#include <iostream>
#include <vector>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MAX_BUF_SIZE 1024

using namespace std;

void get_cmd_stdout(const string& cmd, vector<string>& lines);

int main()
{
    vector<string> lines;

    get_cmd_stdout("lsblk -blp", lines);
    if (lines.size() == 0) {
        cout<<"No output"<<endl;
        return -1;
    }
    vector<string>::iterator iter = lines.begin();
    cout<<"Content:"<<endl;
    for (; iter != lines.end(); iter++){
        // contains newline
        cout<<*iter;
    }
    return 0;
}

void get_cmd_stdout(const string& cmd, vector<string>& lines)
{
    FILE* out;
    char buf[MAX_BUF_SIZE];

    out = popen(cmd.c_str(), "r");
    if (!out) {
        cout<<"Failed to popen: "<<strerror(errno)<<endl;
        return;
    }

    while(!feof(out)) {
        // Notice: the line length should < MAX_BUF_SIZE
        memset(buf, MAX_BUF_SIZE, 0);
        if (fgets(buf, MAX_BUF_SIZE, out) == NULL) {
            continue;
        }
        lines.push_back(string(buf));
    }
    pclose(out);
}
