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

var (
	watchedMountPoint []string
	watchedFileList   []string
)

type eventMap map[string]int64

var (
	rwLocker sync.RWMutex
	evSet    = make(eventMap)

	w *Watcher
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

func watchFile(filename string) error {
	mp := queryFileMountPoint(filename)
	if isItemInList(mp, watchedMountPoint) {
		return nil
	}

	err := w.Watch(filename, unix.FAN_CLASS_CONTENT|unix.FAN_CLOEXEC,
		unix.FAN_MARK_ADD|unix.FAN_MARK_MOUNT, unix.FAN_OPEN_EXEC_PERM|
			unix.FAN_OPEN_PERM|unix.FAN_CLOSE_WRITE)
	if err != nil {
		fmt.Printf("Failed to watch '%v': %v\n", filename, err)
		return err
	}

	fmt.Println("Watched:", filename, mp)
	watchedMountPoint = append(watchedMountPoint, mp)
	watchedFileList = append(watchedFileList, filename)
	return nil
}

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <filename>\n", os.Args[0])
		return
	}

	w, _ = NewWatcher()
	defer w.Close()

	err := watchFile(os.Args[1])
	if err != nil {
		return
	}

	rand.Seed(time.Now().Unix())
	for {
		ev := <-w.Event
		matched := isItemInList(ev.Filename, watchedFileList)
		if ev.MaskMatch(unix.FAN_CLOSE_WRITE) {
			if matched {
				// do something
				fmt.Printf("CLOSE_WRITE: %q <-- %q\n", ev.Filename, ev.Program)
			}
			continue
		}

		hasPerm := false
		if ev.MaskMatch(unix.FAN_OPEN_EXEC_PERM) {
			if !matched {
				ev.Allow()
				continue
			}
			hasPerm = true
			fmt.Printf("OPEN_EXEC_PERM: %q <-- %q -- %v\n", ev.Filename, ev.Program, ev.Pid)
		} else if ev.MaskMatch(unix.FAN_OPEN_PERM) {
			if !matched {
				ev.Allow()
				continue
			}
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
						fmt.Println("\tAllow:", _e.Filename, _e.Program)
						_e.Allow()
					} else {
						fmt.Println("\tDeny:", _e.Filename, _e.Program)
						_e.Deny()
					}
				}(&ev)
			}
		}
	}
}
