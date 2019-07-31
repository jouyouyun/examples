/**
 * Read/Write the memory data by physical address.
 * Such as GPIO pin, details see: https://bob.cs.sonoma.edu/IntroCompOrg-RPi/sec-gpio-mem.html
 * GPIO also can be read/write by sysfs, details see: https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
 **/
package main

import (
	"fmt"
	"os"
	"strconv"
	"syscall"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Printf(`
Usage: %s <address> [data]
    address: the device physical memory address
    data   : data to written

Note: only supported byte
`, os.Args[0])
		return
	}

	addr, err := strconv.ParseUint(os.Args[1], 16, 64)
	if err != nil {
		fmt.Println("Failed to parse address:", err)
		return
	}
	fmt.Printf("Will get data from address: 0x%X\n", addr)

	var data uint64
	if len(os.Args) == 3 {
		data, err = strconv.ParseUint(os.Args[2], 16, 64)
		if err != nil {
			fmt.Println("Failed to parse data:", err)
			return
		}
		fmt.Printf("Will set data(0x%X) for address: 0x%X\n", data, addr)
	}

	fw, err := os.OpenFile("/dev/mem", os.O_RDWR|os.O_SYNC, 0640)
	if err != nil {
		fmt.Println("Failed to open '/dev/mem':", err)
		return
	}
	defer fw.Close()

	pageSize := 4096
	mapMask := pageSize - 1
	off := addr & uint64((^mapMask + 1))
	fmt.Printf("Offset: 0x%X\n", off)
	mapPtr, err := syscall.Mmap(int(fw.Fd()), int64(off), pageSize,
		syscall.PROT_READ|syscall.PROT_WRITE, syscall.MAP_SHARED)
	if err != nil {
		fmt.Println("Failed to mmap:", err)
		return
	}

	virtPtr := &mapPtr[addr&uint64(mapMask)]
	fmt.Printf("Data in address(0x%X): 0x%X\n", addr, *virtPtr)

	if len(os.Args) == 2 {
		goto out
	}

	// write data
	*virtPtr = byte(data)
	fmt.Printf("Write data: 0x%X, read again: 0x%X\n", data, *virtPtr)
out:
	err = syscall.Munmap(mapPtr)
	if err != nil {
		fmt.Println("Failed to munmap:", err)
		return
	}
}
