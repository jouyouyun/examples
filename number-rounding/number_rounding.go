package main

import (
	"fmt"
	"math"
)

func numberRounding(num, round float64) int32 {
	return int32(math.Trunc((num+round)*10) / 10)
}

func main() {
	var numList = []float64{
		1.0,
		1.25,
		1.5,
		1.7,
		1.75,
		2.0,
		2.4,
		2.5,
		2.7,
	}

	fmt.Println("If 1.5 <= num < 2.5, num should be 2")
	for _, num := range numList {
		fmt.Printf("%v after rounding: %v\n", num, numberRounding(num, 0.5))
	}

	fmt.Println("\nIf 1.7 <= num < 2.7, num should be 2")
	for _, num := range numList {
		fmt.Printf("%v after rounding: %v\n", num, numberRounding(num, 0.3))
	}
}
