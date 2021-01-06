// strings to long, inspired by IP2Long
// IP2Long, such as: 1.0.1.1 --> 1<<24 | 0<<16 | 1<<8 | 1 = 16777473
package main

import (
	"flag"
	"fmt"
	"os"
)

const (
	maxLen = 32
)

var (
	str = flag.String("s", "", "the string text")
)

func convertStringToUInt64(data string) uint64 {
	strLen := len(data)
	if strLen <= maxLen {
		return calcBytes([]byte(data), 0)
	}

	i := 0
	idx := 0
	items := []byte(data)
	num := uint64(0)
	for idx < strLen {
		idx = i + maxLen
		if idx > strLen {
			idx = strLen
		}

		tmp := calcBytes(items[i:idx], i)
		num += tmp
		i = idx
	}

	return num
}

func calcBytes(items []byte, idx int) uint64 {
	var num uint64
	for i, v := range items {
		tmp := uint64(v) + uint64(idx)
		tmp <<= i
		num |= uint64(tmp)
	}
	return num
}

func main() {
	flag.Parse()

	strLen := len(*str)
	if strLen == 0 || len(os.Args) == 1 ||
		(len(os.Args) == 2 && (os.Args[1] == "-h" || os.Args[1] == "--help")) {
		flag.Usage()
		return
	}

	// 'aabbccddeeff' == 'aabbeeffccdd' ?
	num := convertStringToUInt64(*str)
	fmt.Printf("%s --> %v\n", *str, num)
}
