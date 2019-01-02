package main

import (
	"fmt"
	x11 "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/util/wm/ewmh"
	"io/ioutil"
)

var (
	conn *x11.Conn
)

func main() {
	var err error
	conn, err = x11.NewConn()
	if err != nil {
		fmt.Println("Failed to connect x11:", err)
		return
	}
	defer conn.Close()

	win, err := ewmh.GetActiveWindow(conn).Reply(conn)
	if err != nil {
		fmt.Println("Failed to get active window:", err)
		return
	}
	dumpWindow(win)
}

func dumpWindow(win x11.Window) {
	fmt.Println("Window:", win)
	name, err := ewmh.GetWMName(conn, win).Reply(conn)
	fmt.Printf("\tName: ")
	if err == nil {
		fmt.Printf("%s\n", name)
	}
	pid, err := ewmh.GetWMPid(conn, win).Reply(conn)
	fmt.Printf("\tPid:")
	if err == nil {
		fmt.Printf("%v\n", pid)
		fmt.Println("\t\tCmdline:", getCmdline(pid))
	}
	icon, err := ewmh.GetWMIconName(conn, win).Reply(conn)
	fmt.Printf("\tIcon: ")
	if err == nil {
		fmt.Printf("%s\n", icon)
	}
	iconList, err := ewmh.GetWMIcon(conn, win).Reply(conn)
	fmt.Printf("\tIcon List: ")
	if err == nil {
		for _, icon := range iconList {
			fmt.Printf("\n\t\t%v", icon.String())
		}
		fmt.Println("")
	}
}

func getCmdline(pid uint32) string {
	data, _ := ioutil.ReadFile(fmt.Sprintf("/proc/%d/cmdline", pid))
	return string(data)
}
