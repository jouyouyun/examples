package main

import (
	"fmt"

	"time"

	"github.com/godbus/dbus"
)

const (
	login1Service = "org.freedesktop.login1"
	login1Path    = "/org/freedesktop/login1/session/self"
	login1IFC     = "org.freedesktop.login1.Session"
)

var (
	conn *dbus.Conn
)

func printActiveSession() {
	obj := conn.Object(login1Service, login1Path)
	value, err := obj.GetProperty(login1IFC + ".Active")
	if err != nil {
		fmt.Println("Failed to get active:", err)
	} else {
		active, ok := value.Value().(bool)
		if ok {
			fmt.Println("Active:", active)
		}
	}

	var strv = []string{"Desktop", "Display", "TTY", "Type"}
	for _, str := range strv {
		value, err = obj.GetProperty(login1IFC + "." + str)
		if err != nil {
			fmt.Printf("Failed to get %s: %s\n", str, err)
		} else {
			v, ok := value.Value().(string)
			if ok {
				fmt.Printf("%s: %s\n", str, v)
			}
		}

	}
}

func main() {
	var err error
	conn, err = dbus.SystemBus()
	if err != nil {
		fmt.Println("Failed to connect dbus:", err)
	}

	for {
		printActiveSession()
		time.Sleep(time.Second * 5)
	}
}
