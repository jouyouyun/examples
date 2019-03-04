package dpkgrepo

import (
	"bufio"
	"compress/gzip"
	"fmt"
	"strconv"
	"strings"
)

type Package struct {
	Name         string
	Distribution string
	Component    string
	Version      string
	Architecture string
	Homepage     string
	Section      string
	Summary      string
	// Priority      string
	// Filename      string
	Size          uint64
	InstalledSize uint64
	// Depends       []string
	// Recommends    []string
	// Suggests      []string
	// Conflicts     []string
	// Replaces      []string
}

type PackageList []*Package

var (
	_selectedKeys = []string{
		"Package",
		"Version",
		"Architecture",
		"Homepage",
		"Section",
		"Description",
		"Size",
		"Installed-Size",
	}
)

func ParseComponent(uri, dist, component, arch string) (PackageList, error) {
	var url = fmt.Sprintf("%s/dists/%s/%s/binary-%s/Packages.gz",
		uri, dist, component, arch)
	resp, err := Download(url)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	gzipReader, err := gzip.NewReader(resp.Body)
	if err != nil {
		return nil, err
	}
	defer gzipReader.Close()

	var scanner = bufio.NewScanner(gzipReader)
	return scanComponentPackages(scanner, dist, component)
}

func scanComponentPackages(scanner *bufio.Scanner,
	dist, component string) (PackageList, error) {
	var repoList PackageList
	var line string
	var set = makeSelectedSet()
	for scanner.Scan() {
		line = scanner.Text()
		if len(line) == 0 {
			// next package
			repoList = append(repoList, &Package{
				Name:          set["Package"],
				Distribution:  dist,
				Component:     component,
				Version:       set["Version"],
				Architecture:  set["Architecture"],
				Homepage:      set["Homepage"],
				Section:       set["Section"],
				Summary:       set["Description"],
				InstalledSize: strToUInt64(set["Installed-Size"]),
				Size:          strToUInt64(set["Size"]),
			})
			set = makeSelectedSet()
			continue
		}
		if line[0] == ' ' {
			// description detailed
			continue
		}
		tmpList := strings.Split(line, ": ")
		_, ok := set[tmpList[0]]
		if ok {
			set[tmpList[0]] = tmpList[1]
		}
	}

	return repoList, nil
}

func makeSelectedSet() map[string]string {
	var set = make(map[string]string)
	for _, key := range _selectedKeys {
		set[key] = ""
	}
	return set
}

func strToUInt64(str string) uint64 {
	n, _ := strconv.ParseUint(str, 10, 64)
	return n
}
