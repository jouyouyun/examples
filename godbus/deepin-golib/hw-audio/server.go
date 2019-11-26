package main

import (
	"fmt"

	dbus "pkg.deepin.io/lib/dbus1"
	"pkg.deepin.io/lib/dbusutil"
)

const (
	dbusService = "com.huawei.audioservice"
	dbusPath    = "/com/huawei/audioservice"
	dbusIFC     = "com.huawei.audioservice.dbus"
)

const (
	WindowTypeClose = iota
	WindowTypeDetail
	WindowTypeSelect
	WindowTypePrompt
)

const (
	DeviceTypeHeadphone = iota
	DeviceTypeHeadset
)

var (
	srv *dbusutil.Service
)

type Audio struct {
	methods *struct {
		SetDeviceType      func() `in:"isAlwaysApply,deviceType" out:"success"`
		SendDisplayRequest func() `in:"windowType,deviceType"`
	}

	signals *struct {
		DisplayRequest struct {
			winType int
			devType int
		}
	}
}

func (*Audio) SetDeviceType(isAlwaysApply bool, deviceType int32) (bool, *dbus.Error) {
	fmt.Println("Will set device type:", isAlwaysApply, deviceType)
	return true, nil
}

func (a *Audio) SendDisplayRequest(windowType, deviceType int32) *dbus.Error {
	err := srv.Emit(a, "DisplayRequest", windowType, deviceType)
	return dbusutil.ToError(err)
}

func (*Audio) GetInterfaceName() string {
	return dbusIFC
}

func main() {
	var err error
	srv, err = dbusutil.NewSystemService()
	if err != nil {
		fmt.Println("Failed to connect dbus:", err)
		return
	}

	owned, err := srv.NameHasOwner(dbusService)
	if err != nil {
		fmt.Println("Failed to query owner:", err)
		return
	}
	if owned {
		fmt.Printf("The service '%s' has been owned\n", dbusService)
		return
	}

	var a = &Audio{}
	err = srv.Export(dbusPath, a)
	if err != nil {
		fmt.Println("Failed to export:", err)
		return
	}

	err = srv.RequestName(dbusService)
	if err != nil {
		fmt.Println("Failed to request name:", err)
		return
	}

	srv.Wait()
}
