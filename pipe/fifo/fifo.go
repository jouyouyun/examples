package main

import (
	"flag"
	"fmt"
	"os"
	"syscall"
)

var (
	mode  = flag.String("mode", "r", "Only read, available: r and w")
	fpath = flag.String("file", "", "The fifo filepath")
)

func main() {
	flag.Parse()
	if len(*fpath) == 0 {
		flag.Usage()
		os.Exit(-1)
	}

	var err error
	if *mode == "r" {
		err = readFifo(*fpath)
	} else {
		err = writeFifo(*fpath, os.Args[len(os.Args)-1])
	}

	if err != nil {
		os.Exit(-1)
	}
}

func readFifo(filepath string) error {
	defer os.Remove(filepath)
	err := syscall.Mkfifo(filepath, 0600)
	if err != nil {
		fmt.Println("Failed to mkfifio:", err)
		return err
	}

	fr, err := os.OpenFile(filepath, os.O_RDONLY, os.ModeNamedPipe)
	if err != nil {
		fmt.Println("Failed to open fifo:", err)
		return err
	}
	defer fr.Close()

	var buf = make([]byte, 1024)
	for {
		_, err = fr.Read(buf)
		if err != nil {
			break
		}
		fmt.Println("Read:", string(buf))
	}
	return nil
}

func writeFifo(filepath, value string) error {
	fw, err := os.OpenFile(filepath, os.O_WRONLY, os.ModeNamedPipe)
	if err != nil {
		fmt.Println("Failed to open fifo:", err)
		return err
	}
	defer fw.Close()

	_, err = fw.Write([]byte(value))
	if err != nil {
		fmt.Println("Failed to write fifo:", err)
	}
	return err
}
