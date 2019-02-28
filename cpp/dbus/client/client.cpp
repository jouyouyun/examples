#include "client.h"

#include <iostream>
#include <vector>
#include <signal.h>

#include <dbus-c++/dbus.h>

using namespace std;

void handle_terminate(int sig);

static const char *DBUS_SERVICE = "org.freedesktop.DBus";
static const char *DBUS_PATH = "/org/freedesktop/DBus";
// static const char *DBUS_IFC = "org.freedesktop.DBus";
static DBus::BusDispatcher dispatcher;

void dump_strv(vector<string>& list, const string& name)
{
    cout<<name<<":"<<endl;
    vector<string>::iterator it = list.begin();
    for (; it != list.end(); it++) {
        cout<<"\t"<<*it<<endl;
    }
}

CallClient::CallClient(DBus::Connection& conn, const char* path,
                       const char* name):DBus::ObjectProxy(conn, path, name)
{
}

void CallClient::NameOwnerChanged(const string& name, const string& old_owner,
                             const string& new_owner)
{
    cout<<"NameOwnerChanged: "<<name<<" - "<<old_owner<<" - "<<new_owner<<endl;
}

void CallClient::NameLost(const string& name)
{
    cout<<"NameLost: "<<name<<endl;
}

void CallClient::NameAcquired(const string& name)
{
    cout<<"NameAcquired: "<<name<<endl;
}

int main()
{
    signal(SIGTERM, handle_terminate);
    signal(SIGINT, handle_terminate);

    DBus::default_dispatcher = &dispatcher;
    DBus::Connection conn = DBus::Connection::SessionBus();

    CallClient client(conn, DBUS_PATH, DBUS_SERVICE);

    vector<string> features = client.Features();
    dump_strv(features, "Features");
    vector<string> ifaces = client.Interfaces();
    dump_strv(ifaces, "Interfaces");

    // Notice: if call exception, program recieved SIGABRT
    try{
    cout<<"Call Hello: "<<client.Hello()<<endl;
    } catch(DBus::Error& e){
        cout<<"Call Hello Error: "<<e.what()<<endl;
    }
    cout<<"Call NameHasOwner: "<<client.NameHasOwner(string(DBUS_SERVICE))<<endl;
    // vector<string> names;
    // names = client.ListNames();
    // dump_strv(names, "Service Names");

    dispatcher.enter();
    return 0;
}

void handle_terminate(int sig)
{
    dispatcher.leave();
}
