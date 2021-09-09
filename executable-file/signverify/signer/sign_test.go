package signer

import (
	"fmt"
	"os"
	"os/exec"
	"testing"
	"time"
)

var countList = []int{1, 5, 10, 50, 100, 200, 500, 1024}

func testPrepare() {
	err := os.Mkdir("testdata", 0755)
	if err != nil {
		fmt.Println("Failed to mkdir:", err)
		os.Exit(-2)
	}

	for i := 0; i < len(countList); i++ {
		err := genTestFile(countList[i], "testdata")
		if err != nil {
			fmt.Println("Failed to generate file:", countList[i], err)
			os.Exit(-1)
		}
	}
}

func testFinalized() {
	err := os.RemoveAll("testdata")
	if err != nil {
		fmt.Println("Failed to remove:", err)
	}
}

func TestSignFile(t *testing.T) {
	testPrepare()
	defer testFinalized()

	var (
		total           = 10
		size            = len(countList)
		average         = make([]time.Duration, size)
		averageFragment = make([]time.Duration, size)
	)
	for j := 0; j < total; j++ {
		fmt.Printf("The %d times\n", j)
		for i := 0; i < size; i++ {
			filename := fmt.Sprintf("testdata/%dM", countList[i])
			prev := time.Now()
			data, err := SignFile(filename)
			duration := time.Now().Sub(prev)
			average[i] += duration
			if err != nil {
				fmt.Println("\tTest failed for SignFile:", filename, err)
			}
			fmt.Printf("\tSignFile:\t%s, \t%s, \tduration: %v\n", filename, string(data), duration)

			prev = time.Now()
			data, err = SignFileByFragment(filename)
			duration = time.Now().Sub(prev)
			averageFragment[i] += duration
			if err != nil {
				fmt.Println("\tTest failed for SignFileByFragment:", filename, err)
			}
			fmt.Printf("\tSignFileByFragment:\t%s, \t%s, duration: %v\n", filename, string(data), duration)
		}
	}

	for i := 0; i < size; i++ {
		fmt.Printf("%d in 10 times average: %v -- %v\n", i, average[i]/time.Duration(total), averageFragment[i]/time.Duration(total))
	}
}

func genTestFile(count int, dir string) error {
	outs, err := exec.Command("dd", "if=/dev/urandom",
		fmt.Sprintf("of=%s/%dM", dir, count), fmt.Sprintf("count=%d", count), "bs=1M").CombinedOutput()
	if err != nil {
		return fmt.Errorf("%s: %s", err, string(outs))
	}
	return nil
}
