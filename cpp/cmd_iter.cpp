#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

#include <boost/algorithm/string.hpp>

using namespace std;

#define MAX_BUF_SIZE 1024

#define CMDLINE "dpkg-query -f '${Package},${Version}\n' -W"
#define CMD_DELIM ","

typedef struct _SoftwareInfo {
    string name;
    string version;
} SoftwareInfo;

class CMDStream {
public:
    CMDStream(const string& cmd);
    ~CMDStream();

    bool Next();

    string name;
    string version;
private:
    FILE *fp;
    char buf[MAX_BUF_SIZE];
};

CMDStream::CMDStream(const string& cmd)
{
    fp = popen(cmd.c_str(), "r");
}

CMDStream::~CMDStream()
{
    if (fp){
        pclose(fp);
    }
}

bool CMDStream::Next()
{
    if (!fp || feof(fp)) {
        cout<<"Open failed failed or file eof"<<endl;
        return false;
    }

    memset(buf, 0, MAX_BUF_SIZE);
    if (fgets(buf, MAX_BUF_SIZE, fp) == NULL) {
        // read failure
        cout<<"Read file failed"<<endl;
        return false;
    }

    vector<string> list;
    namespace al = boost::algorithm;
    string line = string(buf);
    al::trim_right_if(line, al::is_any_of("\n"));
    al::split(list, line, al::is_any_of(CMD_DELIM));
    if (list.size() != 2) {
        cout<<"Invalid data"<<list.size()<<", line:"<<line<<endl;
        // invalid data
        return false;
    }
    name = list[0];
    version = list[1];
    return true;
}

int main()
{
    int count = 0;
    CMDStream cs(CMDLINE);
    while(cs.Next()) {
        if (count > 10) {
            break;
        }
        cout<<"Name: "<<cs.name<<", version: "<<cs.version<<endl;
        count++;
    }
    return 0;
}
