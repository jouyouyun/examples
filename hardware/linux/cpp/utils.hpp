#ifndef __UTILS_UTILS_H__
#define __UTILS_UTILS_H__

#include <iostream>
#include <vector>

using namespace std;

#define TRIM_LEFT 1 << 0
#define TRIM_RIGHT 1 << 1

void SplitString(const string& str, vector<string>& list,
                 const string& delim, const int num = -1);
string& TrimString(string& str, const string delim = " ",
                   const int mode = TRIM_LEFT|TRIM_RIGHT);

#endif
