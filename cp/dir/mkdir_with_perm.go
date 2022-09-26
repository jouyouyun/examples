package main

import (
	"fmt"
	"io/fs"
	"os"
	"syscall"
)

func IsExists(filename string) bool {
	_, err := os.Stat(filename)
	if err == nil || os.IsExist(err) {
		return true
	}
	return false
}

func Mkdir(srcDir, dstDir string) error {
	if IsExists(dstDir) {
		return nil
	}

	fi, err := os.Stat(srcDir)
	if err != nil {
		return err
	}
	fmt.Printf("%s mode: %v\n", srcDir, fi.Mode())
	err = os.MkdirAll(dstDir, fi.Mode())
	if err != nil {
		return err
	}

	// set uid and gid
	if stat, ok := fi.Sys().(*syscall.Stat_t); ok {
		dstMode := fi.Mode().Perm()
		changed := false
		// man 7 inode
		if stat.Mode&syscall.S_ISUID == syscall.S_ISUID {
			fmt.Println("has suid")
			dstMode |= fs.ModeSetuid
			changed = true
		}
		if stat.Mode&syscall.S_ISGID == syscall.S_ISGID {
			fmt.Println("has sgid")
			dstMode |= fs.ModeSetgid
			changed = true
		}
		if stat.Mode&syscall.S_ISVTX == syscall.S_ISVTX {
			fmt.Println("has vtx")
			dstMode |= fs.ModeSticky
			changed = true
		}
		if changed {
			err = os.Chmod(dstDir, dstMode)
			fmt.Println("chmod again:", err)
		}
		err = os.Lchown(dstDir, int(stat.Uid), int(stat.Gid))
		if err != nil {
			return err
		}
	} else {
		return fmt.Errorf("failed to get raw stat for: %s", srcDir)
	}

	return nil
}

func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <src dir> <dst dir>\n", os.Args[0])
		return
	}

	err := Mkdir(os.Args[1], os.Args[2])
	if err != nil {
		fmt.Println(err)
	}
}
