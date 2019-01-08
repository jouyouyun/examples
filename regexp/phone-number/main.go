package main

import (
	"fmt"
	"regexp"
)

const (
	rule = `^1[3-9](\d{9})$`
)

func main() {
	req := regexp.MustCompile(rule)
	var list = []string{
		"13298074578",
		"15327053378",
		"13609678123",
		"16675412314",
		"19823211212",
		"1982321212",
		"198232121211",
		"11109819001",
		"132880923-1",
	}
	for _, v := range list {
		fmt.Println(v, "\t:", req.MatchString(v))
	}
}
