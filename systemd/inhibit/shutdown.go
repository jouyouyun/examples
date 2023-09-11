package main

import (
	"fmt"
	"os"
	"syscall"

	"github.com/godbus/dbus"
)

type inhibitorInfo struct {
	Name    string
	App     string
	Desc    string
	Action  string
	Timeout uint32
	FD      uint32
}
type inhibitorInfos []inhibitorInfo

const (
	logindService = "org.freedesktop.login1"
	logindPath    = "/org/freedesktop/login1"
	logindIFC     = "org.freedesktop.login1.Manager"

	logindMethodInhibit     = "Inhibit"
	logindMethodListInhibit = "ListInhibitors"
	logindSignalShutdown    = "PrepareForShutdown"
)

var (
	_conn *dbus.Conn = nil
)

func main() {
	c, err := dbus.SystemBus()
	if err != nil {
		fmt.Println("[ERR] connect system bus:", err)
		os.Exit(-1)
	}

	_conn = c
	defer _conn.Close()

	fd := uint32(0)
	err = _conn.Object(logindService, logindPath).Call(
		logindIFC+"."+logindMethodInhibit, 0,
		"shutdown", "inhibit_test", "Inhibit Test Shutdown", "block").Store(&fd)
	if err != nil {
		fmt.Println("[ERR] Inhibit:", err)
		os.Exit(-1)
	}

	listInhibitors()
	monitorShutdown()

	err = syscall.Close(int(fd))
	if err != nil {
		fmt.Println("[ERR] close fd:", err)
		os.Exit(-1)
	}
}

func listInhibitors() {
	var infos inhibitorInfos
	err := _conn.Object(logindService, logindPath).Call(
		logindIFC+"."+logindMethodListInhibit, 0).Store(&infos)
	if err != nil {
		fmt.Println("[ERR] list inhibitors:", err)
		return
	}

	fmt.Println("Inhibitors")
	for _, info := range infos {
		fmt.Printf("%#v\n", info)
	}
}

func monitorShutdown() {
	sigChan := make(chan *dbus.Signal)
	rule := fmt.Sprintf("type='signal', path='%s', interface='%s', member='%s'",
		logindPath, logindIFC, logindSignalShutdown)

	_conn.BusObject().Call("org.freedesktop.DBus.AddMatch", 0, rule)
	_conn.Signal(sigChan)
	for {
		sig, ok := <-sigChan
		if !ok {
			fmt.Println("Except signal")
			break
		}

		fmt.Println("Recieved signal:", sig.Path, sig.Name)
		if sig.Name != logindIFC+"."+logindSignalShutdown {
			continue
		}
		v := sig.Body[0].(bool)
		fmt.Println("Signal Value:", v)
		if v {
			break
		}
	}
}
