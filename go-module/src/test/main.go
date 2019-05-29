package main

import (
	"github.com/godbus/dbus"
	"github.com/golang/glog"
)

func main() {
	conn, err := dbus.SessionBus()
	if err != nil {
		glog.Error("Failed to connect bus:", err)
		return
	}
	obj := conn.Object("org.freedesktop.DBus", "/")
	var ret string
	err = obj.Call("org.freedesktop.DBus.Hello", 0).Store(&ret)
	if err != nil {
		glog.Error("Failed to call:", err)
		return
	}
	glog.Info("Result:", ret)
}
