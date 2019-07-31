package main

import (
	"fmt"

	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

var (
	conn *x.Conn
	smbs *keysyms.KeySymbols
)

func getKeycodes(key string) ([]x.Keycode, error) {
	fmt.Println("Will convert to keycodes:", key)
	codes, err := smbs.StringToKeycodes(key)
	if err != nil {
		fmt.Println("Failed to convert keycodes:", err)
		return nil, err
	}
	fmt.Println("Keycodes:", codes)
	return codes, nil
}

func main() {
	var err error
	conn, err = x.NewConn()
	if err != nil {
		fmt.Println("Failed to connect x:", err)
		return
	}
	smbs = keysyms.NewKeySymbols(conn)
	keys := []string{
		"XF86AudioPlay",
		"XF86AudioPause",
	}
	for _, key := range keys {
		getKeycodes(key)
	}
}
