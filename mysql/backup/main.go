package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
)

const (
	pidFilename = "/tmp/mysql_backup_helper.pid"
	progName    = "mysql_backup_helper"
)

var (
	config = flag.String("c", "config.json", "the configuration file path")
	test   = flag.Bool("t", false, "test it")
)

func main() {
	flag.Parse()
	if hasRunning() {
		fmt.Println("[Info] There has a backup helper running, exit...")
		return
	}

	err := writePidFile()
	if err != nil {
		fmt.Println("[Error] Failed to write pid file:", err)
		return
	}

	conf, err := loadConfig(*config)
	if err != nil {
		fmt.Println("[Error] Failed to load config:", err)
		return
	}

	bak, err := NewBackup(conf)
	if err != nil {
		fmt.Println("[Error] Failed to new backup:", err)
		return
	}

	bak.Clean()
	bak.StartLoop()
}

func hasRunning() bool {
	contents, err := ioutil.ReadFile(pidFilename)
	if err != nil {
		return false
	}
	if len(contents) == 0 {
		return false
	}
	pid, err := strconv.ParseInt(string(contents), 10, 64)
	if err != nil {
		return false
	}
	contents, err = ioutil.ReadFile(fmt.Sprintf("/proc/%d/cmdline", pid))
	if err != nil {
		return false
	}
	return strings.Contains(string(contents), progName)
}

func writePidFile() error {
	return ioutil.WriteFile(pidFilename,
		[]byte(fmt.Sprint(os.Getpid())), 0644)
}
