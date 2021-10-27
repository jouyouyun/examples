package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

var (
	mountPointBlacklist = []string{
		"/proc",
		"/sys",
		"/sys",
		"/dev",
		"/run",
	}

	_mountPointList []string
)

func init() {
	list, err := parseMountPoint("/proc/self/mountinfo")
	if err != nil {
		panic(err)
	}
	_mountPointList = list
}

func queryFileMountPoint(filename string) string {
	var mp string
	for _, v := range _mountPointList {
		if strings.Contains(filename, v) && len(mp) < len(v) {
			mp = v
		}
	}
	return mp
}

func parseMountPoint(filename string) ([]string, error) {
	fr, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer fr.Close()

	var mounts []string
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		items := strings.SplitN(line, " ", 11)
		if len(items) != 11 {
			fmt.Println("Failed to split:", line)
			continue
		}
		if !isMountPointBlacklist(items[4]) {
			mounts = append(mounts, items[4])
		}
	}

	return mounts, nil
}

func isMountPointBlacklist(mp string) bool {
	for _, v := range mountPointBlacklist {
		if (v == mp) || (strings.Contains(mp, v) && mp[len(v)] == '/') {
			return true
		}
	}
	return false
}

func isItemInList(item string, list []string) bool {
	for _, v := range list {
		if v == item {
			return true
		}
	}
	return false
}
