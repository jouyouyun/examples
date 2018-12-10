package main

import (
	"fmt"
	"regexp"
	"strconv"
	"strings"
)

func main() {
	input := "01010100011001010111001101110100"
	r, _ := regexp.Compile("[0|1]{8}")

	match := r.FindAllString(input, -1)
	b := make([]byte, 0)
	for _, s := range match {
		n, _ := strconv.ParseUint(s, 2, 8)
		b = append(b, byte(n))
	}
	fmt.Println("Test1:", string(b))

	test2()
}

func test2() {
	fmt.Printf("Test2: ")
	input := "01010100 01100101 01110011 01110100"
	for _, v := range strings.Fields(input) {
		n, _ := strconv.ParseUint(v, 2, 8)
		fmt.Printf("%s", string(byte(n)))
	}
	fmt.Println()
}
