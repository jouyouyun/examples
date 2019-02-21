#ifndef __UTILS_FILE_H__
#define __UTILS_FILE_H__

#include <fstream>

using namespace std;

class File {
public:
    File(const string& filepath, ios::openmode mode);
    ~File();
    string GetContents();
    string GetKey(const string& key, const string delim = ":",
                  const int num = 2);
private:
    fstream fs;
};

#endif
