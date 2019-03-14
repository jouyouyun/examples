package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <lshw json file>\n", os.Args[0])
		return
	}

	content, err := ioutil.ReadFile(os.Args[1])
	if err != nil {
		fmt.Println("Failed to read file:", err)
		return
	}

	hw, err := NewLshw(content)
	if err != nil {
		fmt.Println("Failed to new lshw:", err)
		return
	}
	network := hw.GetNetwork()
	if len(network) != 0 {
		network.Dump()
	}

	disk := hw.GetDisk()
	if len(disk) != 0 {
		disk.Dump()
	}
}
