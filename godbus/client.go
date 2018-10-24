package main

import (
	"fmt"

	"github.com/godbus/dbus"
)

const (
	dbusName = "org.jouyouyun.Test"
	dbusPath = "/org/jouyouyun/Test"
	dbusIFC  = "org.jouyouyun.Test"
)

var (
	conn *dbus.Conn
)

func main() {
	var err error
	conn, err = dbus.SessionBus()
	if err != nil {
		fmt.Println("[Client] Failed to connection bus:", err)
		return
	}

	callMethod()
	getProperty()
	monitorSignal()
}

func callMethod() {
	method := dbusIFC + ".Hello"
	var ret string
	err := conn.Object(dbusName, dbusPath).Call(method, 0).Store(&ret)
	if err != nil {
		fmt.Println("[Client] Failed to call 'Hello':", err)
		return
	}
	fmt.Println("[Client] 'Hello' result:", ret)
}

func getProperty() {
	variant, err := conn.Object(dbusName, dbusPath).GetProperty(dbusIFC + ".Name")
	if err != nil {
		fmt.Println("[Client] Failed to get property:", err)
		return
	}
	fmt.Println("[Client] Prop 'Name':", variant.Value().(string))
}

func monitorSignal() {
	var rule = fmt.Sprintf("type='signal',path='%s',interface='%s',member='NameChanged'",
		dbusPath, dbusIFC)
	conn.BusObject().Call("org.freedesktop.DBus.AddMatch", 0, rule)
	var c = make(chan *dbus.Signal)
	conn.Signal(c)
	for {
		sig, ok := <-c
		if !ok {
			break
		}
		fmt.Println("[Client] Signal:", sig.Path, sig.Name)
		if sig.Name != dbusIFC+".NameChanged" {
			continue
		}
		fmt.Println("[Client] Signal Value:", sig.Body[0].(string))
		getProperty()
	}
}
