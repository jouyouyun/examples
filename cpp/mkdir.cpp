#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout<<"Usage: "<<argv[0]<<" <file>"<<endl;
        return 0;
    }

    namespace bf = boost::filesystem;
    bf::path dir(argv[1]);
    cout<<"Dir: "<<dir.parent_path()<<endl;

    if (bf::exists(dir.parent_path())) {
        return 0;
    }

    if (!bf::create_directories(dir.parent_path())) {
        cout<<"Create dir failed"<<endl;
        return -1;
    }

    return 0;
}
