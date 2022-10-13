package main

import (
	"fmt"
	"os"
	"syscall"
	"unsafe"
)

func ioctl(fd, request, argp uintptr) error {
	_, _, e := syscall.Syscall6(syscall.SYS_IOCTL, fd, request, argp, 0, 0, 0)
	if e != 0 {
		return e
	}
	return nil
}

func tcGetAttr(fd uintptr, argp *syscall.Termios) error {
	return ioctl(fd, syscall.TCGETS, uintptr(unsafe.Pointer(argp)))
}

func tcSetAttr(fd, action uintptr, argp *syscall.Termios) error {
	return ioctl(fd, action, uintptr(unsafe.Pointer(argp)))
}

func main() {
	var term syscall.Termios
	var origin syscall.Termios

	fd := os.Stdin.Fd()
	err := tcGetAttr(fd, &term)
	if err != nil {
		fmt.Println(err)
		return
	}

	fmt.Printf("Please input:")
	origin = term
	// echo off
	term.Lflag &= ^(uint32(syscall.ICANON | syscall.ECHO))
	// term.Cc[syscall.VMIN] = 1
	// term.Cc[syscall.VTIME] = 0
	tcSetAttr(fd, syscall.TCSETS, &term)
	var buf string
	fmt.Scanf("%s", &buf)

	// echo on
	tcSetAttr(fd, syscall.TCSETS, &origin)

	fmt.Println("\nRead:", buf)
}
