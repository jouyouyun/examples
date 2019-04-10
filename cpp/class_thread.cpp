#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace std;

class Core {
public:
     virtual void Println(const string &data) = 0;
};

class Test {
public:
    Test();
    ~Test();

    void Println(const string &data);

    int id;
    boost::thread_group grp;
};

Test::Test()
{
    id = 5;
}

Test::~Test()
{
    this->grp.join_all();
}

void Test::Println(const string &data)
{
    cout<<"ID: "<<this->id<<", data: "<<data<<endl;
}

int main()
{
    Test t;
    t.grp.create_thread(boost::bind(&Test::Println, &t, "Test1"));
    t.grp.create_thread(boost::bind(&Test::Println, &t, "Test1"));
    return 0;
}
