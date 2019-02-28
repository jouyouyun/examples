#pragma once

#include "dbus-gule.h"
#include <ios>
#include <dbus-c++/dbus.h>

class CallClient: public org::freedesktop::DBus_proxy,
                  public DBus::IntrospectableProxy,
                  public DBus::ObjectProxy {
public:
    CallClient(DBus::Connection& conn, const char* path, const char* name);

    void NameOwnerChanged(const std::string& name, const std::string& old_owner,
                          const std::string& new_owner);
    void NameLost(const std::string& name);
    void NameAcquired(const std::string& name);
};
