package main

import (
	"fmt"
	"math/rand"
	"os"
	"sync"
	"time"

	"golang.org/x/sys/unix"
)

const (
	defaultEventDuration = 3
)

type eventMap map[string]int64

var (
	rwLocker sync.RWMutex
	evSet    = make(eventMap)
)

func (set eventMap) Get(key string) int64 {
	rwLocker.RLock()
	v := set[key]
	rwLocker.RUnlock()
	return v
}

func (set eventMap) Set(key string, timestamp int64) {
	rwLocker.Lock()
	set[key] = timestamp
	rwLocker.Unlock()
}

func (set eventMap) Update(key string) bool {
	prev := set.Get(key)
	timestamp := time.Now().Unix()
	if timestamp-prev < defaultEventDuration {
		return false
	}
	set.Set(key, timestamp)
	return true
}

func genEventMapKey(info *EventInfo) string {
	return fmt.Sprintf("%v-%s-%s", info.Pid, info.Program, info.Filename)
}

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <filename>\n", os.Args[0])
		return
	}
	w, _ := NewWatcher()
	defer w.Close()

	err := w.Watch(os.Args[1], unix.FAN_CLASS_CONTENT|unix.FAN_CLOEXEC,
		unix.FAN_MARK_ADD, unix.FAN_OPEN_EXEC_PERM|unix.FAN_OPEN_PERM)
	if err != nil {
		fmt.Println("Failed to watch 'hostname':", err)
		return
	}

	rand.Seed(time.Now().Unix())
	for {
		ev := <-w.Event
		if ev.MaskMatch(unix.FAN_CLOSE_WRITE) {
			fmt.Printf("CLOSE_WRITE: %q <-- %q\n", ev.Filename, ev.Program)
		}

		hasPerm := false
		if ev.MaskMatch(unix.FAN_OPEN_EXEC_PERM) {
			hasPerm = true
			fmt.Printf("OPEN_EXEC_PERM: %q <-- %q -- %v\n", ev.Filename, ev.Program, ev.Pid)
		} else if ev.MaskMatch(unix.FAN_OPEN_PERM) {
			hasPerm = true
			fmt.Printf("OPEN_PERM: %q <-- %q -- %v\n", ev.Filename, ev.Program, ev.Pid)
		}
		if hasPerm {
			key := genEventMapKey(&ev)
			updated := evSet.Update(key)
			if !updated {
				ev.Allow()
			} else {
				go func(_e *EventInfo) {
					if rand.Int31n(20)%2 == 0 {
						_e.Allow()
					} else {
						_e.Deny()
					}
				}(&ev)
			}
		}
	}
}
