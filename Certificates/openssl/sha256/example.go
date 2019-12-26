package main

import (
	"crypto/sha256"
	"fmt"
	"os"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <data>\n", os.Args[0])
		return
	}

	var h = sha256.New()
	h.Write([]byte(os.Args[1]))
	var d = h.Sum(nil)

	fmt.Printf("Digest len: %d, %x\n", len(d), d)
}
