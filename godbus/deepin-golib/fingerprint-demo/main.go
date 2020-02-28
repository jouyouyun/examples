package main

import (
	"fmt"

	"pkg.deepin.io/lib/dbusutil"
)

func main() {
	// TODO: single pattern

	srv, err := dbusutil.NewSystemService()
	if err != nil {
		fmt.Println("Failed to connect dbus:", err)
		return
	}

	d, err := newDemo(srv)
	if err != nil {
		fmt.Println("Failed to new demo:", err)
		return
	}

	err = srv.Export(dbusPath, d)
	if err != nil {
		fmt.Println("Failed to export object:", err)
		return
	}

	err = srv.RequestName(dbusService)
	if err != nil {
		srv.StopExport(d)
		fmt.Println("Failed to request name:", err)
		return
	}

	srv.Wait()
}
