#include "file.hpp"

#include <vector>
#include <exception>

#include <boost/algorithm/string.hpp>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            void StringSplit(vector<string>& list, const string& str,
                             const string& delim, const int num = 2);

            FilePrivate::FilePrivate(const string& filepath, ios::openmode mode)
            {
                fstream *tmp = new fstream;
                try {
                    tmp->open(filepath, mode);
                } catch (exception& e) {
                    cout<<"Failed to open file:"<< e.what()<<endl;
                    return;
                }

                fp = tmp;
            }

            FilePrivate::~FilePrivate()
            {
                if (!fp) {
                    return;
                }
                fp->close();
                delete fp;
            }

            string FilePrivate::LoadContent()
            {
                if (!fp) {
                    return string("");
                }

                istreambuf_iterator<char> begin(*fp);
                istreambuf_iterator<char> end;
                string content;
                try {
                    content = string(begin, end);
                }  catch (exception& e) {
                    cout<<"Failed to load file content:"<<e.what()<<endl;
                }

                return content;
            }

            string FilePrivate::GetKey(const string& key, const string& delim)
            {
                if (!fp) {
                    return string("");
                }

                try {
                    fp->seekp(ios::beg);
                } catch (exception& e) {
                    cout<<"Failed to seek file:"<<e.what()<<endl;
                    return string("");
                }

                string result;
                string line;
                while(getline(*fp, line)) {
                    vector<string> list;
                    StringSplit(list, line, delim);
                    if (list.size() != 2) {
                        continue;
                    }
                    boost::algorithm::trim(list[0]);
                    if (list[0] != key) {
                        continue;
                    }
                    result = list[1];
                    break;
                }
                boost::algorithm::trim_left_if(result,
                                          boost::algorithm::is_any_of("\t"));
                return result;
            }

            void StringSplit(vector<string>& list, const string& str,
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
        } // namespace systeminfo
    } // namespace module
} // namesapce dmcg
