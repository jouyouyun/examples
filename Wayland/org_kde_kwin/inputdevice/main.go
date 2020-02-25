package main

import (
	"fmt"

	"strconv"
	"strings"

	kwin "github.com/linuxdeepin/go-dbus-factory/org.kde.kwin"
	dbus "pkg.deepin.io/lib/dbus1"
)

const (
	eventPathPrefix = "/org/kde/KWin/InputDevice/"
)

type DevType int32

const (
	DevTypeKbd DevType = iota + 1
	DevTypeMouse
	DevTypeTouchpad
)

type deviceInfo struct {
	ID      int32
	Type    DevType
	Enabled bool
	Name    string
}

func (t DevType) String() string {
	switch t {
	case DevTypeKbd:
		return "keyboard"
	case DevTypeMouse:
		return "mouse"
	case DevTypeTouchpad:
		return "touchpad"
	}
	return "unknown"
}

var (
	_conn *dbus.Conn
)

func newInputDevice(sysName string) (*deviceInfo, error) {
	dev, err := kwin.NewInputDevice(_conn, dbus.ObjectPath(eventPathPrefix+sysName))
	if err != nil {
		fmt.Println("Failed to new input device:", err)
		return nil, err
	}
	dumpInputDevice(dev)

	var info deviceInfo
	kbd, _ := dev.Keyboard().Get(0)
	numKbd, _ := dev.AlphaNumericKeyboard().Get(0)
	if kbd && numKbd {
		info.Type = DevTypeKbd
		goto fill
	}

	if tpad, _ := dev.Touchpad().Get(0); tpad {
		info.Type = DevTypeTouchpad
		goto fill
	}

	{
		fcount, _ := dev.TapFingerCount().Get(0)
		suppLeftHanded, _ := dev.SupportsLeftHanded().Get(0)
		suppBtns, _ := dev.SupportedButtons().Get(0)
		sbtn, _ := dev.ScrollButton().Get(0)
		if (fcount == 0) && suppLeftHanded && (suppBtns > 0) && (sbtn != 0) {
			info.Type = DevTypeMouse
			goto fill
		}
	}

	return nil, nil

fill:
	id, _ := strconv.Atoi(strings.Split(sysName, "event")[1])
	info.ID = int32(id)
	info.Name, _ = dev.Name().Get(0)
	info.Enabled, _ = dev.Enabled().Get(0)

	return &info, nil
}

func dumpInputDevice(dev *kwin.InputDevice) {
	name, _ := dev.Name().Get(0)
	fmt.Printf("%s\n", name)
	enabled, _ := dev.Enabled().Get(0)
	fmt.Println("\tEnabled:", enabled)
	sysName, _ := dev.SysName().Get(0)
	fmt.Println("\tSysName:", sysName)
	vendor, _ := dev.Vendor().Get(0)
	fmt.Println("\tVendor:", vendor)
	product, _ := dev.Product().Get(0)
	fmt.Println("\tProduct:", product)
	kbd, _ := dev.Keyboard().Get(0)
	fmt.Println("\tIs Keybaord:", kbd)
	numKbd, _ := dev.AlphaNumericKeyboard().Get(0)
	fmt.Println("\tIs Number Keyboard:", numKbd)
	tpad, _ := dev.Touchpad().Get(0)
	fmt.Println("\tIs Touchpad:", tpad)
	touch, _ := dev.Touch().Get(0)
	fmt.Println("\tCan Touch:", touch)
	tabTool, _ := dev.TabletTool().Get(0)
	fmt.Println("\tIs Tablet Tool:", tabTool)
	tms, _ := dev.TabletModeSwitch().Get(0)
	fmt.Println("\tTablet Mode Switch:", tms)
	cma, _ := dev.ClickMethodAreas().Get(0)
	fmt.Println("\tClick Method Areas:", cma)
	dcma, _ := dev.DefaultClickMethodAreas().Get(0)
	fmt.Println("\tDefault Click Method Areas:", dcma)
	cmcf, _ := dev.ClickMethodClickfinger().Get(0)
	fmt.Println("\tClick Method Click Finger:", cmcf)
	dcmcf, _ := dev.DefaultClickMethodClickfinger().Get(0)
	fmt.Println("\tDefault Click Method Click Finger:", dcmcf)
	paccel, _ := dev.PointerAcceleration().Get(0)
	fmt.Println("\tPointer Acceleration:", paccel)
	dpaccel, _ := dev.DefaultPointerAcceleration().Get(0)
	fmt.Println("\tDefault Pointer Acceleration:", dpaccel)
	paccelpa, _ := dev.PointerAccelerationProfileAdaptive().Get(0)
	fmt.Println("\tPointer Acceleration Profile Adaptive:", paccelpa)
	dpaccelpa, _ := dev.DefaultPointerAccelerationProfileAdaptive().Get(0)
	fmt.Println("\tDefault Pointer Acceleration Profile Adaptive:", dpaccelpa)
	paccelpf, _ := dev.PointerAccelerationProfileFlat().Get(0)
	fmt.Println("\tPointer Acceleration Profile Flat:", paccelpf)
	dpaccelpf, _ := dev.DefaultPointerAccelerationProfileFlat().Get(0)
	fmt.Println("\tDefault Pointer Acceleration Profile Flat:", dpaccelpf)
	sbtn, _ := dev.ScrollButton().Get(0)
	fmt.Println("\tScroll Button:", sbtn)
	dsbtn, _ := dev.DefaultScrollButton().Get(0)
	fmt.Println("\tDefault Scroll Button:", dsbtn)
	dwtyping, _ := dev.DisableWhileTyping().Get(0)
	fmt.Println("\tDisable While Typing:", dwtyping)
	dwtyinged, _ := dev.DisableWhileTypingEnabledByDefault().Get(0)
	fmt.Println("\tDisable While Typing Enabled Default:", dwtyinged)
	gestureSupp, _ := dev.GestureSupport().Get(0)
	fmt.Println("\tGesture Support:", gestureSupp)
	leftHanded, _ := dev.LeftHanded().Get(0)
	fmt.Println("\tLeft Handed:", leftHanded)
	leftHandedD, _ := dev.LeftHandedEnabledByDefault().Get(0)
	fmt.Println("\tLeft Handed Default:", leftHandedD)
	lidS, _ := dev.LidSwitch().Get(0)
	fmt.Println("\tLid Switch:", lidS)
	lmrTapBtnMap, _ := dev.LmrTapButtonMap().Get(0)
	fmt.Println("\tLmr Tap Button Map:", lmrTapBtnMap)
	lmrTapBtnMapD, _ := dev.LmrTapButtonMapEnabledByDefault().Get(0)
	fmt.Println("\tLmr Tap Button Map Enable Default:", lmrTapBtnMapD)
	midEmu, _ := dev.MiddleEmulation().Get(0)
	fmt.Println("\tMiddle Emulation:", midEmu)
	midEmuD, _ := dev.MiddleEmulationEnabledByDefault().Get(0)
	fmt.Println("\tMiddle Emulation Default:", midEmuD)
	natureS, _ := dev.NaturalScroll().Get(0)
	fmt.Println("\tNature Scroll:", natureS)
	natureSD, _ := dev.NaturalScrollEnabledByDefault().Get(0)
	fmt.Println("\tNature Scroll Default:", natureSD)
	outName, _ := dev.OutputName().Get(0)
	fmt.Println("\tOutput Name:", outName)
	sedge, _ := dev.ScrollEdge().Get(0)
	fmt.Println("\tScroll Edge:", sedge)
	sedgeD, _ := dev.ScrollEdgeEnabledByDefault().Get(0)
	fmt.Println("\tScroll Edge Default:", sedgeD)
	sobtnd, _ := dev.ScrollOnButtonDown().Get(0)
	fmt.Println("\tScroll On Button Down:", sobtnd)
	sobtndD, _ := dev.ScrollOnButtonDownEnabledByDefault().Get(0)
	fmt.Println("\tScroll On Button Down Default:", sobtndD)
	stfinger, _ := dev.ScrollTwoFinger().Get(0)
	fmt.Println("\tScroll Two Finger:", stfinger)
	stfingerD, _ := dev.ScrollTwoFingerEnabledByDefault().Get(0)
	fmt.Println("\tScroll Two Finger Default:", stfingerD)
	suppBtns, _ := dev.SupportedButtons().Get(0)
	fmt.Println("\tSupported Buttons:", suppBtns)
	suppCaliMatrix, _ := dev.SupportsCalibrationMatrix().Get(0)
	fmt.Println("\tSupported Calibration Matrix:", suppCaliMatrix)
	suppClickMA, _ := dev.SupportsClickMethodAreas().Get(0)
	fmt.Println("\tSupported Click Method Areas:", suppClickMA)
	suppClickMCf, _ := dev.SupportsClickMethodClickfinger().Get(0)
	fmt.Println("\tSupported Click Method Clickfinger:", suppClickMCf)
	suppDisWTyping, _ := dev.SupportsDisableWhileTyping().Get(0)
	fmt.Println("\tSupport Disable While Typing:", suppDisWTyping)
	suppLeftHanded, _ := dev.SupportsLeftHanded().Get(0)
	fmt.Println("\tSupport Left Handed:", suppLeftHanded)
	suppLmrTapBtn, _ := dev.SupportsLmrTapButtonMap().Get(0)
	fmt.Println("\tSupport Lmr Tap Button Map:", suppLmrTapBtn)
	suppMidEmu, _ := dev.SupportsMiddleEmulation().Get(0)
	fmt.Println("\tSupport Middle Emulation:", suppMidEmu)
	suppNaturalS, _ := dev.SupportsNaturalScroll().Get(0)
	fmt.Println("\tSupport Natural Scroll:", suppNaturalS)
	suppPAccel, _ := dev.SupportsPointerAcceleration().Get(0)
	fmt.Println("\tSupport Pointer Acceleration:", suppPAccel)
	suppPAccelP, _ := dev.SupportsPointerAccelerationProfileAdaptive().Get(0)
	fmt.Println("\tSupport Pointer Acceleration Profile Adaptive:", suppPAccelP)
	suppPAccelPF, _ := dev.SupportsPointerAccelerationProfileFlat().Get(0)
	fmt.Println("\tSupport Pointer Acceleration Profile Flat:", suppPAccelPF)
	suppSEdge, _ := dev.SupportsScrollEdge().Get(0)
	fmt.Println("\tSupport Scroll Edge:", suppSEdge)
	suppSOBtn, _ := dev.SupportsScrollOnButtonDown().Get(0)
	fmt.Println("\tSupport Scroll On Button Down:", suppSOBtn)
	suppSTFinger, _ := dev.SupportsScrollTwoFinger().Get(0)
	fmt.Println("\tSupport Scroll Two Finger:", suppSTFinger)
	tad, _ := dev.TapAndDrag().Get(0)
	fmt.Println("\tTap And Drag:", tad)
	tadD, _ := dev.TapAndDragEnabledByDefault().Get(0)
	fmt.Println("\tTap And Drag Default:", tadD)
	tdl, _ := dev.TapDragLock().Get(0)
	fmt.Println("\tTap Drag Lock:", tdl)
	tdlD, _ := dev.TapDragLockEnabledByDefault().Get(0)
	fmt.Println("\tTap Drag Lock Default:", tdlD)
	ttc, _ := dev.TapToClick().Get(0)
	fmt.Println("\tTap To Click:", ttc)
	ttcD, _ := dev.TapToClickEnabledByDefault().Get(0)
	fmt.Println("\tTap To Click Default:", ttcD)
	tapFinger, _ := dev.TapFingerCount().Get(0)
	fmt.Println("\tTap Finger Count:", tapFinger)
}

func newKbdDevice(dev *kwin.InputDevice) *deviceInfo {
	var info = deviceInfo{Type: DevTypeKbd}
	info.Enabled, _ = dev.Enabled().Get(0)

	return &info
}

func newTouchpadDevice(dev *kwin.InputDevice) *deviceInfo {
	var info = deviceInfo{Type: DevTypeTouchpad}
	info.Enabled, _ = dev.Enabled().Get(0)

	return &info
}

func newMouseDevice(dev *kwin.InputDevice) *deviceInfo {
	var info = deviceInfo{Type: DevTypeMouse}
	info.Enabled, _ = dev.Enabled().Get(0)

	return &info
}

func main() {
	fmt.Println("Start")
	conn, err := dbus.SessionBus()
	if err != nil {
		fmt.Println("Failed to connect session bus:", err)
		return
	}
	_conn = conn

	var manager = kwin.NewInputDeviceManager(conn)
	//_, err = manager.ConnectDeviceAdded(func(sysName string){
	//	fmt.Println("Device added:", sysName)
	//})
	//if err != nil {
	//	fmt.Println("Failed to listen device added:", err)
	//	return
	//}
	//_, err = manager.ConnectDeviceRemoved(func(sysName string){
	//	fmt.Println("Device removed:", sysName)
	//})
	//if err != nil {
	//	fmt.Println("Failed to listen device added:", err)
	//	return
	//}

	var infos []*deviceInfo
	sysNames, _ := manager.DevicesSysNames().Get(0)
	for _, sysName := range sysNames {
		info, err := newInputDevice(sysName)
		if err != nil {
			fmt.Println("Failed to new input device:", err)
			continue
		}
		if info != nil {
			infos = append(infos, info)
		}
	}

	fmt.Println("Device Infos:")
	for _, info := range infos {
		fmt.Println("\t", info.ID, info.Name, info.Enabled, info.Type.String())
	}
}
