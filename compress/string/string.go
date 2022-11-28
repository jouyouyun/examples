package main

import (
	"crypto/sha256"
	"encoding/base64"
	"encoding/hex"
	"fmt"

	"github.com/emmansun/gmsm/sm3"
)

var (
	// 0 ~ 93
	_charList = []byte{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'`', '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_', '=', '+', '{', '}', '[', ']', '\\', '|', ':', ';', '\'', '"', ',', '<', '.', '>', '/', '?',
	}

	modNum4       = 4
	modNum4Offset = 2
	modNum4Max    = 3
)

// mod(4): --> 3 char
//   1  1  1 1 1 1
//   32 16 8 4 2 1 = 32+31 = 63

func calMod(b, mod byte) byte {
	return b % mod
}

func calDivision(b, mod byte) byte {
	return b / mod
}

func compressBlock(b []byte, mod, maxNum, offset byte) ([]byte, error) {
	if len(b) > int(maxNum) {
		return nil, fmt.Errorf("invalid block: %s, too long, max %d", b, maxNum)
	}

	ret := []byte{}
	ret = append(ret, byte(0))
	for i, v := range b {
		m := calMod(v, mod)
		d := calDivision(v, mod)
		ret[0] |= m << (i * int(offset))
		ret = append(ret, _charList[d])
	}
	idx := ret[0]
	if int(idx) > len(_charList) {
		return nil, fmt.Errorf("invalid mod index: %d, too long", idx)
	}
	ret[0] = _charList[idx]
	return ret, nil
}

func compressData(data []byte, mod, maxNum, offset byte) ([]byte, error) {
	i := 0
	target := []byte{}
	dataLen := len(data)
	for i < dataLen {
		end := i + int(maxNum)
		if end > dataLen {
			end = dataLen
		}
		bbytes, err := compressBlock(data[i:end], mod, maxNum, offset)
		if err != nil {
			return nil, err
		}
		target = append(target, bbytes...)
		i += int(maxNum)
	}

	return target, nil
}

func expandData(data []byte) []byte {
	target := []byte{}
	for _, v := range data {
		h := v >> 4
		l := v & 0x0f
		target = append(target, h)
		target = append(target, l)
	}
	return target
}

func main() {
	hash := sha256.Sum256([]byte("Hello, world!"))
	fmt.Printf("SHA256: \n%s\n", hex.EncodeToString(hash[:]))

	mod4Bytes, err := compressData(hash[:],
		byte(modNum4), byte(modNum4Max), byte(modNum4Offset))
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(string(mod4Bytes))

	sm3Hash := sm3.Sum([]byte("Hello, world!"))

	fmt.Printf("SM3:\n%s\n", hex.EncodeToString(sm3Hash[:]))
	mod4Bytes, err = compressData(sm3Hash[:],
		byte(modNum4), byte(modNum4Max), byte(modNum4Offset))
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(string(mod4Bytes))

	sm3Hash2 := expandData(sm3Hash[:])
	mod2Bytes, err := compressData(sm3Hash2, 2, 6, 1)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(string(mod2Bytes))

	fmt.Println(base64.RawStdEncoding.EncodeToString(mod2Bytes))
}
