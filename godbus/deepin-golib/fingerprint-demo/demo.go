package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
	"time"

	dbus "pkg.deepin.io/lib/dbus1"
	"pkg.deepin.io/lib/dbusutil"
)

const (
	dbusService = "com.deepin.Fingerprint.Demo"
	dbusPath    = "/com/deepin/Fingerprint/Demo"
	dbusIFC     = "com.deepin.Fingerprint"

	fingerprintStoreDir = "/var/lib/deepin-fingerprint-demo/templates"
)

const (
	devStateNormal  int32 = 1 << 0
	devStateClaimed       = 1 << 1
)

const (
	devTypeUnknown int32 = iota
	devTypeScanning
	devTypeTouch
)

const (
	devCapAutoLogin int32 = 1 << 1
)

var (
	errTypeArgs      = fmt.Errorf("invalid arguments")
	errTypeAbnormal  = fmt.Errorf("device abnormal")
	errTypeClaimed   = fmt.Errorf("device has been claimed")
	errTypeUnClaimed = fmt.Errorf("device not claimed")
)

type opWorking struct {
	locker  sync.RWMutex
	working bool
	exit    chan bool
}

func (op *opWorking) Get() bool {
	op.locker.RLock()
	v := op.working
	op.locker.RUnlock()
	return v
}

func (op *opWorking) Set(v bool) {
	op.locker.Lock()
	op.working = v
	op.locker.Unlock()
}

func (op *opWorking) Exit() {
	op.locker.RLock()
	v := op.working
	op.locker.RUnlock()
	if !v {
		return
	}
	op.exit <- true
}

type Demo struct {
	srv *dbusutil.Service

	enrollWorking *opWorking
	verifyWorking *opWorking

	curID string

	stateLocker sync.RWMutex

	Name       string
	Type       int32
	State      int32
	Capability int32

	signals *struct {
		EnrollStatus struct {
			id   string
			code int32
			msg  string
		}

		VerifyStatus struct {
			id   string
			code int32
			msg  string
		}
	}

	methods *struct {
		Claim            func() `in:"id,cliamed"`
		Enroll           func() `in:"fingerName"`
		StopEnroll       func()
		Verify           func() `in:"fingerName"`
		StopVerify       func()
		DeleteFinger     func() `in:"id,fingerName"`
		DeleteAllFingers func() `in:"id"`
		ListFingers      func() `in:"id" out:"fingerNameList"`
	}
}

func newDemo(srv *dbusutil.Service) (*Demo, error) {
	var d = Demo{
		srv: srv,
		enrollWorking: &opWorking{
			working: false,
			exit:    make(chan bool),
		},
		verifyWorking: &opWorking{
			working: false,
			exit:    make(chan bool),
		},
		Name:       "deepin-demo",
		Type:       devTypeUnknown,
		State:      devStateNormal,
		Capability: devCapAutoLogin,
	}

	return &d, nil
}

func (d *Demo) isDevNormal() bool {
	d.stateLocker.RLock()
	defer d.stateLocker.RUnlock()
	return (d.State&devStateNormal == devStateNormal)
}

func (d *Demo) isDevClaimed() bool {
	d.stateLocker.RLock()
	defer d.stateLocker.RUnlock()
	return (d.State&devStateClaimed == devStateClaimed)
}

func (d *Demo) canEnrollOrVerify() *dbus.Error {
	if !d.isDevNormal() {
		return dbusutil.ToError(errTypeAbnormal)
	}
	if !d.isDevClaimed() {
		return dbusutil.ToError(errTypeUnClaimed)
	}
	return nil
}

var _count = 0

func (d *Demo) enrollHandler(name string) {
	var i = 0
	for {
		if i >= 3 {
			break
		}
		select {
		case <-time.NewTimer(time.Second).C:
			var code int32
			var msg string
			switch i {
			case 0:
				code = 2
				msg = "{\"progress\":30}"
			case 1:
				code = 3
				msg = "{\"subcode\":1}"
			case 2:
				code = 3
				msg = "{\"subcode\":3}"
			}
			_ = d.srv.Emit(d, "EnrollStatus", d.curID, code, msg)
			i++
		case <-d.enrollWorking.exit:
			_ = d.srv.Emit(d, "EnrollStatus", d.curID, 1, "{\"subcode\":3}")
			return
		}
	}

	if _count%2 == 0 {
		_count = 1
		_ = touchFingerFile(d.curID, name)
		_ = d.srv.Emit(d, "EnrollStatus", d.curID, 2, "{\"progress\":100}")
		_ = d.srv.Emit(d, "EnrollStatus", d.curID, 0, "")
	} else {
		_count = 0
		_ = d.srv.Emit(d, "EnrollStatus", d.curID, 1, "{\"subcode\":2}")
	}
	d.enrollWorking.Set(false)
}

func (d *Demo) verifyHandler(name string) {
	var i = 0
	for {
		if i >= 3 {
			break
		}
		select {
		case <-time.NewTimer(time.Second).C:
			var msg string
			switch i {
			case 0:
				msg = "{\"subcode\":1}"
			case 1:
				msg = "{\"subcode\":2}"
			case 2:
				msg = "{\"subcode\":3}"
			}
			_ = d.srv.Emit(d, "VerifyStatus", d.curID, 3, msg)
			i++
		case <-d.verifyWorking.exit:
			_ = d.srv.Emit(d, "VerifyStatus", d.curID, 2, "{\"subcode\":1}")
			return
		}
	}

	if _count%2 == 0 {
		_count = 1
		_ = d.srv.Emit(d, "VerifyStatus", d.curID, 0, "")
	} else {
		_count = 0
		_ = d.srv.Emit(d, "VerifyStatus", d.curID, 1, "")
	}
	d.verifyWorking.Set(false)
}

func (*Demo) GetInterfaceName() string {
	return dbusIFC
}

func isFingerExist(id, finger string) bool {
	_, err := os.Stat(filepath.Join(fingerprintStoreDir, id, finger))
	if err != nil {
		return false
	}
	return true
}

func touchFingerFile(id, finger string) error {
	file := filepath.Join(fingerprintStoreDir, id, finger)
	err := os.MkdirAll(filepath.Dir(file), 0700)
	if err != nil {
		return err
	}

	return ioutil.WriteFile(file, []byte("demo"), 0600)
}
