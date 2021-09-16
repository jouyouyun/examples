package signer

import (
	"fmt"
	"io/ioutil"
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

	testSignFile()
	testVerifyFile()
}

func testSignFile() {
	var (
		total        = 10
		size         = len(countList)
		average      = make([]time.Duration, size)
		averageChunk = make([]time.Duration, size)
	)
	for j := 0; j < total; j++ {
		fmt.Printf("[Sign] The %d times\n", j)
		var result string = ""
		var resultChunk string = ""
		for i := 0; i < size; i++ {
			filename := fmt.Sprintf("testdata/%dM", countList[i])
			prev := time.Now()
			data, err := SignFile(filename)
			duration := time.Now().Sub(prev)
			average[i] += duration
			if err != nil {
				fmt.Println("\tTest failed for SignFile:", filename, err)
			}
			result += fmt.Sprintf(" & %v", duration)
			if j != 0 {
				writeFile(filename, data, false)
			}

			prev = time.Now()
			data, err = SignFileByChunk(filename)
			duration = time.Now().Sub(prev)
			averageChunk[i] += duration
			if err != nil {
				fmt.Println("\tTest failed for SignFileByChunk:", filename, err)
			}
			resultChunk += fmt.Sprintf(" & %v", duration)
			if j != 0 {
				writeFile(filename, data, true)
			}
		}
		fmt.Println("SignFile\t:", result)
		fmt.Println("SignFileByChunk\t:", resultChunk)
	}

	var result string = ""
	var resultChunk string = ""
	for i := 0; i < size; i++ {
		result += fmt.Sprintf(" & %v", average[i]/time.Duration(total))
		resultChunk += fmt.Sprintf(" & %v", averageChunk[i]/time.Duration(total))
	}
	fmt.Println("SignFile Average\t:", result)
	fmt.Println("SignFileByChunk Average\t:", resultChunk)
}

func testVerifyFile() {
	var (
		total        = 10
		size         = len(countList)
		average      = make([]time.Duration, size)
		averageChunk = make([]time.Duration, size)
	)
	for j := 0; j < total; j++ {
		fmt.Printf("[Verify] The %d times\n", j)
		var result string = ""
		var resultChunk string = ""
		for i := 0; i < size; i++ {
			filename := fmt.Sprintf("testdata/%dM", countList[i])
			prev := time.Now()
			ok, err := VerifyFile(filename, filename+".sign")
			duration := time.Now().Sub(prev)
			if err != nil || !ok {
				fmt.Println("\tTest failed for verify file:", filename, err)
				continue
			}
			result += fmt.Sprintf(" & %v", duration)
			average[i] += duration

			prev = time.Now()
			ok, err = VerifyFileByChunk(filename, filename+".sign.chunk")
			duration = time.Now().Sub(prev)
			if err != nil || !ok {
				fmt.Println("\tTest failed for verify file:", filename, err)
				continue
			}
			resultChunk += fmt.Sprintf(" & %v", duration)
			averageChunk[i] += duration
		}
		fmt.Println("VerifyFile\t:", result)
		fmt.Println("VerifyFileByChunk\t:", resultChunk)
	}

	var result string = ""
	var resultChunk string = ""
	for i := 0; i < size; i++ {
		result += fmt.Sprintf(" & %v", average[i]/time.Duration(total))
		resultChunk += fmt.Sprintf(" & %v", averageChunk[i]/time.Duration(total))
	}
	fmt.Println("VerifyFile Average\t:", result)
	fmt.Println("VerifyFileByChunk Average\t:", resultChunk)
}

func genTestFile(count int, dir string) error {
	outs, err := exec.Command("dd", "if=/dev/urandom",
		fmt.Sprintf("of=%s/%dM", dir, count), fmt.Sprintf("count=%d", count), "bs=1M").CombinedOutput()
	if err != nil {
		return fmt.Errorf("%s: %s", err, string(outs))
	}
	return nil
}

func writeFile(filename string, data []byte, chunk bool) {
	var suffix = ".sign"
	if chunk {
		suffix += ".chunk"
	}
	_ = ioutil.WriteFile(filename+suffix, data, 0644)
}
