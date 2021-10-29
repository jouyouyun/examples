package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"os"
	"sync"

	"golang.org/x/sys/unix"
)

type fileInfo struct {
	fd     int
	fp     *os.File
	reader *bufio.Reader
}

type EventInfo struct {
	Filename string
	Program  string
	File     *fileInfo
	unix.FanotifyEventMetadata
}

type Watcher struct {
	locker  sync.RWMutex
	fileSet map[string]*fileInfo

	Event chan EventInfo
}

func (ev *EventInfo) MaskMatch(mask uint64) bool {
	return (ev.Mask&mask == mask)
}

func (ev *EventInfo) Close() error {
	return unix.Close(int(ev.Fd))
}

func (ev *EventInfo) Allow() error {
	return binary.Write(ev.File.fp, binary.LittleEndian, &unix.FanotifyResponse{
		Fd:       ev.Fd,
		Response: unix.FAN_ALLOW,
	})
}

func (ev *EventInfo) Deny() error {
	return binary.Write(ev.File.fp, binary.LittleEndian, &unix.FanotifyResponse{
		Fd:       ev.Fd,
		Response: unix.FAN_DENY,
	})
}

func NewWatcher() (*Watcher, error) {
	var w = &Watcher{
		fileSet: make(map[string]*fileInfo),
		Event:   make(chan EventInfo, 1024),
	}
	return w, nil
}

func (w *Watcher) Watch(filename string, initFlags, markFlags, eventFlags uint) error {
	if w.isFileExists(filename) {
		return fmt.Errorf("Has exists: %s", filename)
	}

	fd, err := unix.FanotifyInit(initFlags, unix.O_RDONLY|unix.O_LARGEFILE)
	if err != nil {
		return err
	}
	err = unix.FanotifyMark(fd, markFlags, uint64(eventFlags), unix.AT_FDCWD, filename)
	if err != nil {
		_ = unix.Close(fd)
		return err
	}

	w.addFile(filename, fd)
	go w.loop(filename)
	return nil
}

func (w *Watcher) Remove(filename string) error {
	if !w.isFileExists(filename) {
		return fmt.Errorf("Not found: %s", filename)
	}

	fi := w.removeFile(filename)
	fi.fp.Close()
	return nil
}

func (w *Watcher) Close() {
	w.locker.Lock()
	defer w.locker.Unlock()
	for _, v := range w.fileSet {
		v.fp.Close()
	}
	close(w.Event)
}

func (w *Watcher) loop(filename string) error {
	var err error
	fi := w.getFileInfo(filename)
	for {
		var event unix.FanotifyEventMetadata
		err = binary.Read(fi.reader, binary.LittleEndian, &event)
		if err != nil && err != unix.EAGAIN {
			fmt.Println("Failed to read:", err)
			break
		}

		if event.Vers != unix.FANOTIFY_METADATA_VERSION {
			err = fmt.Errorf("Mismatch version")
			break
		}

		file, _ := os.Readlink(fmt.Sprintf("/proc/self/fd/%d", event.Fd))
		progPath, _ := os.Readlink(fmt.Sprintf("/proc/%d/exe", event.Pid))
		w.Event <- EventInfo{
			Filename:              file,
			Program:               progPath,
			File:                  fi,
			FanotifyEventMetadata: event,
		}
	}

	w.Remove(filename)
	return err
}

func (w *Watcher) isFileExists(filename string) bool {
	w.locker.RLock()
	_, ok := w.fileSet[filename]
	w.locker.RUnlock()
	return ok
}

func (w *Watcher) addFile(filename string, fd int) {
	fp := os.NewFile(uintptr(fd), "")
	w.locker.Lock()
	w.fileSet[filename] = &fileInfo{
		fd:     fd,
		fp:     fp,
		reader: bufio.NewReader(fp),
	}
	w.locker.Unlock()
}

func (w *Watcher) removeFile(filename string) *fileInfo {
	w.locker.Lock()
	fi := w.fileSet[filename]
	delete(w.fileSet, filename)
	w.locker.Unlock()
	return fi
}

func (w *Watcher) getFileInfo(filename string) *fileInfo {
	w.locker.RLock()
	info := w.fileSet[filename]
	w.locker.RUnlock()
	return info
}
