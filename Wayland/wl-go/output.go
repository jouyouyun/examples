package main

import (
	"fmt"

	"time"

	"github.com/dkolbly/wl"
)

// Manager wayland protocol manager
type Manager struct {
	disp *wl.Display
	reg  *wl.Registry
	out  *wl.Output

	outInfos OutputInfos
}

// OutputInfo wayland output info
type OutputInfo struct{}
type OutputInfos []*OutputInfo

// OutputGeometryInfo wayland output geometry info
type OutputGeometryInfo struct{}

// OutputModeInfo wayland output mode info
type OutputModeInfo struct{}
type OutputModeInfos []*OutputModeInfo

var (
	_manager *Manager
)

func main() {
	m := GetGlobalOutputInfo("")
	if m == nil {
		return
	}

	_, _ = m.QueryOutputList(true)

	cb, err := m.disp.Sync()
	if err != nil {
		fmt.Println("Failed to sync:", err)
		return
	}

	cdeChan := make(chan wl.CallbackDoneEvent)
	cdeHandler := doner{cdeChan}
	cb.AddDoneHandler(cdeHandler)

	go func() {
		time.Sleep(time.Second * 10)
		err := m.out.Context().SendRequest(m.out, 3, int32(1))
		if err != nil {
			fmt.Println("Failed to send output scale request:", err)
			return
		}

		err = m.out.Context().SendRequest(m.out, 2)
		if err != nil {
			fmt.Println("Failed to send output scale request:", err)
			return
		}
		_, err = m.disp.Sync()
		if err != nil {
			fmt.Println("Failed to sync:", err)
		}
	}()

loop:
	for {
		select {
		case m.disp.Context().Dispatch() <- struct{}{}:
		case <-cdeChan:
			break loop
		}
	}

	time.Sleep(time.Second * 20)
}

func GetGlobalOutputInfo(addr string) *Manager {
	if _manager != nil {
		return _manager
	}

	disp, err := wl.Connect(addr)
	if err != nil {
		fmt.Println("Failed to connect wayland:", err)
		return nil
	}

	_manager = &Manager{
		disp: disp,
	}

	return _manager
}

type registrar struct {
	ch chan wl.RegistryGlobalEvent
}

func (r registrar) HandleRegistryGlobal(ev wl.RegistryGlobalEvent) {
	r.ch <- ev
}

type doner struct {
	ch chan wl.CallbackDoneEvent
}

func (d doner) HandleCallbackDone(ev wl.CallbackDoneEvent) {
	d.ch <- ev
}

// QueryOutputList query wayland output info list
func (m *Manager) QueryOutputList(force bool) (OutputInfos, error) {
	if !force && len(m.outInfos) != 0 {
		return m.outInfos, nil
	}

	reg, err := m.disp.GetRegistry()
	if err != nil {
		fmt.Println("Failed to get registry:", err)
		return nil, err
	}

	cb, err := m.disp.Sync()
	if err != nil {
		fmt.Println("Failed to sync:", err)
		return nil, err
	}

	rgeChan := make(chan wl.RegistryGlobalEvent)
	rgeHandler := registrar{rgeChan}
	m.reg = reg
	reg.AddGlobalHandler(rgeHandler)

	cdeChan := make(chan wl.CallbackDoneEvent)
	cdeHandler := doner{cdeChan}
	cb.AddDoneHandler(cdeHandler)

loop:
	for {
		select {
		case ev := <-rgeChan:
			fmt.Println("[Event] global:", ev.Name, ev.Interface, ev.Version)
			switch ev.Interface {
			case "wl_output":
				m.handleOutputEvent(ev)
			}
		case m.disp.Context().Dispatch() <- struct{}{}:
		case <-cdeChan:
			break loop
		}
	}

	fmt.Println("Will remove global handler")

	reg.RemoveGlobalHandler(m)
	cb.RemoveDoneHandler(cdeHandler)

	if m.out == nil {
		fmt.Println("Output is not registered")
		return nil, nil
	}

	return m.outInfos, nil
}

func (m *Manager) HandleRegistryGlobal(ev wl.RegistryGlobalEvent) {
	switch ev.Interface {
	case "wl_output":
		fmt.Println("[Event] [Gloabl] output:", ev.Name, ev.Interface, ev.Version)
		m.handleOutputEvent(ev)
	}
}

type outputDoner struct {
	ch chan wl.OutputDoneEvent
}

func (d outputDoner) HandleOutputDone(ev wl.OutputDoneEvent) {
	d.ch <- ev
}

func (m *Manager) handleOutputEvent(ev wl.RegistryGlobalEvent) {
	// var version = ev.Version
	// if version > 2 {
	// 	version = 2
	// }

	out := wl.NewOutput(m.disp.Context())
	m.out = out

	err := m.reg.Bind(ev.Name, ev.Interface, ev.Version, out)
	if err != nil {
		fmt.Println("[handleOutputEvent] failed to bind registry:", err)
		return
	}

	// outDoneChan := make(chan wl.OutputDoneEvent)
	// outDoner := outputDoner{outDoneChan}
	out.AddGeometryHandler(m)
	out.AddModeHandler(m)
	out.AddScaleHandler(m)
	out.AddDoneHandler(m)
}

func (m *Manager) HandleOutputGeometry(ev wl.OutputGeometryEvent) {
	fmt.Println("[Output] geometry:", ev.X, ev.Y, ev.PhysicalWidth, ev.PhysicalHeight, ev.Subpixel,
		ev.Make, ev.Model, ev.Transform)
}

func (m *Manager) HandleOutputMode(ev wl.OutputModeEvent) {
	fmt.Println("[Output] mode:", ev.Width, ev.Height, ev.Refresh/1000.0,
		ev.Flags&wl.OutputModeCurrent, ev.Flags&wl.OutputModePreferred)
}

func (m *Manager) HandleOutputDone(ev wl.OutputDoneEvent) {
	fmt.Println("[Output] Done")
}

func (m *Manager) HandleOutputScale(ev wl.OutputScaleEvent) {
	fmt.Println("[Output] scale:", ev.Factor)
}
