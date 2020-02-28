package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"

	dbus "pkg.deepin.io/lib/dbus1"
	"pkg.deepin.io/lib/dbusutil"
)

func (d *Demo) Claim(id string, claimed bool) *dbus.Error {
	if !d.isDevNormal() {
		return dbusutil.ToError(errTypeAbnormal)
	}
	if claimed {
		if d.isDevClaimed() {
			return dbusutil.ToError(errTypeClaimed)
		}
	} else {
		if !d.isDevClaimed() {
			return dbusutil.ToError(errTypeUnClaimed)
		}
	}

	d.stateLocker.Lock()
	if claimed {
		d.State |= devStateClaimed
		d.curID = id
	} else {
		d.State ^= devStateClaimed
		d.curID = ""
	}
	d.stateLocker.Unlock()

	fmt.Println("Device state:", d.State)
	// TODO: emit 'State' changed
	return nil
}

func (d *Demo) Enroll(fingerName string) *dbus.Error {
	if err := d.canEnrollOrVerify(); err != nil {
		return err
	}

	if len(fingerName) == 0 {
		return dbusutil.ToError(errTypeArgs)
	}

	d.enrollWorking.Set(true)
	go d.enrollHandler(fingerName)
	return nil
}

func (d *Demo) StopEnroll() *dbus.Error {
	if err := d.canEnrollOrVerify(); err != nil {
		return err
	}

	if !d.enrollWorking.Get() {
		return dbusutil.ToError(fmt.Errorf("no enroll in process"))
	}

	d.enrollWorking.Exit()
	d.enrollWorking.Set(false)
	return nil
}

func (d *Demo) Verify(fingerName string) *dbus.Error {
	if err := d.canEnrollOrVerify(); err != nil {
		return err
	}

	if len(fingerName) == 0 {
		return dbusutil.ToError(errTypeArgs)
	}

	names, _ := d.ListFingers(d.curID)
	if len(names) == 0 {
		return dbusutil.ToError(fmt.Errorf("no any finger has enrolled"))
	}

	if !isFingerExist(d.curID, fingerName) {
		return dbusutil.ToError(fmt.Errorf("this finger has not enrolled"))
	}

	d.verifyWorking.Set(true)
	go d.verifyHandler(fingerName)
	return nil
}

func (d *Demo) StopVerify() *dbus.Error {
	if err := d.canEnrollOrVerify(); err != nil {
		return err
	}

	if !d.verifyWorking.Get() {
		return dbusutil.ToError(fmt.Errorf("no verify in process"))
	}

	d.verifyWorking.Exit()
	d.verifyWorking.Set(false)
	return nil
}

func (d *Demo) DeleteFinger(id, fingerName string) *dbus.Error {
	if d.enrollWorking.Get() || d.verifyWorking.Get() {
		return dbusutil.ToError(fmt.Errorf("Has a enroll or verify in process"))
	}

	file := filepath.Join(fingerprintStoreDir, id, fingerName)
	err := os.Remove(file)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (d *Demo) DeleteAllFingers(id string) *dbus.Error {
	if d.enrollWorking.Get() || d.verifyWorking.Get() {
		return dbusutil.ToError(fmt.Errorf("Has a enroll or verify in process"))
	}

	dir := filepath.Join(fingerprintStoreDir, id)
	err := os.RemoveAll(dir)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (*Demo) ListFingers(id string) ([]string, *dbus.Error) {
	dir := filepath.Join(fingerprintStoreDir, id)
	finfos, err := ioutil.ReadDir(dir)
	if err != nil {
		return nil, dbusutil.ToError(err)
	}

	var names []string
	for _, finfo := range finfos {
		if finfo.IsDir() {
			continue
		}
		names = append(names, finfo.Name())
	}

	return names, nil
}
