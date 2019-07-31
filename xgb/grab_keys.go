package main

import (
	"fmt"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/keybind"
	"github.com/BurntSushi/xgbutil/xevent"
)

type keycodeGrabMap map[xproto.Keycode]bool

var (
	_keycodeSet = make(keycodeGrabMap)
)

func doGrab(xu *xgbutil.XUtil, key string) error {
	fmt.Println("Start grab:", key)
	mod, codes, err := keybind.ParseString(xu, key)
	if err != nil {
		return err
	}

	for _, code := range codes {
		// if code == 172 && key == "XF86AudioPause" {
		// continue
		// }
		if _, ok := _keycodeSet[code]; ok {
			fmt.Println("Keycode has been grabbed:", key, code)
			continue
		}
		fmt.Println("Do grab:", key, mod, code)
		err := keybind.GrabChecked(xu, xu.RootWin(), mod, code)
		if err != nil {
			fmt.Println("========Grab error:", key, mod, code, err)
			continue
		}
		_keycodeSet[code] = true
	}
	return nil
}

func main() {
	xu, err := xgbutil.NewConn()
	if err != nil {
		fmt.Println("New xgbutils failed:", err)
		return
	}
	keybind.Initialize(xu)

	var keys = []string{
		"mod4-q",
		"XF86AudioPlay",
		"XF86AudioPause",
	}
	for _, key := range keys {
		err := doGrab(xu, key)
		if err != nil {
			fmt.Println("Grab failed:", key, err)
		}
	}

	xevent.KeyPressFun(
		func(x *xgbutil.XUtil, ev xevent.KeyPressEvent) {
			fmt.Println("Recieve event:", ev.State, ev.Detail)
			modStr := keybind.ModifierString(ev.State)
			tmp := keybind.LookupString(xu, ev.State, ev.Detail)
			fmt.Println("Shortcut:", modStr, tmp)
		}).Connect(xu, xu.RootWin())
	xevent.Main(xu)
}
