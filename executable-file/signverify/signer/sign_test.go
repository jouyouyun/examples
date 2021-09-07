package signer

import (
	"fmt"
	"math/rand"
	"os"
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
		fw, err := os.OpenFile(fmt.Sprintf("testdata/%dM", countList[i]), os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0600)
		if err != nil {
			fmt.Println("Failed to open file:", countList[i], err)
			os.Exit(-1)
		}

		for j := 0; j < countList[i]; j++ {
			fw.Write(make1MData())
		}
		fw.Close()
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
			data, err = SignFileWithFragment(filename)
			duration = time.Now().Sub(prev)
			averageFragment[i] += duration
			if err != nil {
				fmt.Println("\tTest failed for SignFileWithFragment:", filename, err)
			}
			fmt.Printf("\tSignFileWithFragment:\t%s, \t%s, duration: %v\n", filename, string(data), duration)
		}
	}

	for i := 0; i < size; i++ {
		fmt.Printf("%d in 10 times average: %v -- %v\n", i, average[i]/time.Duration(total), averageFragment[i]/time.Duration(total))
	}
}

func make1MData() []byte {
	var size = 1024 * 1024
	var str = []byte("0123456789~!@#$%^&*()-=_+qwertyuiopasdfghjklmnbvcxz[];',./<>?:{}")
	var strLen = len(str)
	var data = make([]byte, size)

	rand.Seed(time.Now().Unix())
	for i := 0; i < size; i++ {
		idx := rand.Intn(strLen)
		data[i] = str[idx]
	}

	return data
}
