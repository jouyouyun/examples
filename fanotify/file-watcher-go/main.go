package main

import (
	"fmt"
	"math/rand"
	"time"

	"golang.org/x/sys/unix"
)

func main() {
	w, _ := NewWatcher()
	defer w.Close()

	err := w.Watch("/usr/local/bin/hostname", unix.FAN_CLASS_CONTENT, unix.FAN_MARK_ADD,
		unix.FAN_OPEN_EXEC_PERM)
	if err != nil {
		fmt.Println("Failed to watch 'hostname':", err)
		return
	}
	err = w.Watch("/home/wen/deepin.xsettingsd", unix.FAN_CLOEXEC, unix.FAN_MARK_ADD,
		unix.FAN_CLOSE_WRITE)
	if err != nil {
		fmt.Println("Failed to watch 'xsettingsd':", err)
	}

	go func() {
		time.Sleep(time.Second * 15)
		err := w.Remove("/usr/local/bin/hostname")
		if err != nil {
			fmt.Println("Failed to remove 'hostname':", err)
		}
	}()

	rand.Seed(time.Now().Unix())
	for {
		ev := <-w.Event
		if ev.MaskMatch(unix.FAN_CLOSE_WRITE) {
			fmt.Printf("CLOSE_WRITE: %q <-- %q\n", ev.Filename, ev.Program)
		} else if ev.MaskMatch(unix.FAN_OPEN_EXEC_PERM) {
			fmt.Printf("OPEN_EXEC_PERM: %q <-- %q\n", ev.Filename, ev.Program)
			if rand.Int31n(20)%2 == 0 {
				ev.Allow()
			} else {
				ev.Deny()
			}
		}
	}
}
