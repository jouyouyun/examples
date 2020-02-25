package main

import (
	"fmt"
	"os"

	"github.com/dkolbly/wl"
	"pkg.deepin.io/dde/api/kwayland/org_kde_kwin/dpms"
)

var (
	done bool
)

type dpmsHandler struct {
	modeCh       chan dpms.DpmsModeEvent
	doneCh       chan dpms.DpmsDoneEvent
	globalDoneCh chan wl.CallbackDoneEvent
}

func (h *dpmsHandler) HandleDpmsMode(ev dpms.DpmsModeEvent) {
	h.modeCh <- ev
}

func (h *dpmsHandler) HandleDpmsDone(ev dpms.DpmsDoneEvent) {
	h.doneCh <- ev
}

func (h *dpmsHandler) HandleCallbackDone(ev wl.CallbackDoneEvent) {
	h.globalDoneCh <- ev
}

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <on/off>\n", os.Args[0])
		return
	}

	disp, err := wl.Connect("")
	if err != nil {
		fmt.Println("Failed to connect wayland server:", err)
		return
	}

	var handler = dpmsHandler{
		modeCh:       make(chan dpms.DpmsModeEvent),
		doneCh:       make(chan dpms.DpmsDoneEvent),
		globalDoneCh: make(chan wl.CallbackDoneEvent),
	}

	cb, err := disp.Sync()
	if err != nil {
		fmt.Println("Failed to sync:", err)
	}
	cb.AddDoneHandler(&handler)

	var mode uint32 = dpms.DpmsModeOn
	switch os.Args[1] {
	case "on":
		mode = dpms.DpmsModeOn
	case "off":
		mode = dpms.DpmsModeOff
	default:
		fmt.Println("Unknown dpms mode")
		return
	}

	dpmsManager := dpms.NewDpmsManager(disp.Context())

	dpmsObj := dpms.NewDpms(dpmsManager.Context())
	dpmsObj.AddModeHandler(&handler)
	dpmsObj.AddDoneHandler(&handler)

	err = dpmsObj.Set(mode)
	if err != nil {
		fmt.Println("Failed to set dpms:", err)
	}

loop:
	for {
		select {
		case dpmsManager.Context().Dispatch() <- struct{}{}:
		case ev := <-handler.modeCh:
			fmt.Println("DPMS mode changed:", ev.Mode)
		case <-handler.doneCh:
			fmt.Println("DPMS done event")
			break loop
		case <-handler.globalDoneCh:
			fmt.Println("Wayland done event")
			break loop
		}
	}
}
