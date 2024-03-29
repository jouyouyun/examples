package signer

import (
	"bytes"
	"crypto/sha256"
	"io"
	"io/ioutil"
	"os"
	"runtime"
	"sync"

	"github.com/panjf2000/ants"
)

const (
	defaultChunkSize = 1024 * 1024
	sizePerRead      = 1024
)

func SignFile(filename string) ([]byte, error) {
	fr, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer fr.Close()

	var h = sha256.New()
	if _, err := io.Copy(h, fr); err != nil {
		return nil, err
	}

	return h.Sum(nil), nil
}

func SignFileByChunk(filename string) ([]byte, error) {
	list, err := calcChunkHash(filename)
	if err != nil {
		return nil, err
	}

	var encoder = sha256.New()
	for i := 0; i < len(list); i++ {
		_, _ = encoder.Write(list[i][:])
	}

	return encoder.Sum(nil), nil
}

func VerifyFile(fpath, hashFile string) (bool, error) {
	signature, err := SignFile(fpath)
	if err != nil {
		return false, err
	}
	return verifySignature(hashFile, signature)
}

func VerifyFileByChunk(fpath, hashFile string) (bool, error) {
	signature, err := SignFileByChunk(fpath)
	if err != nil {
		return false, err
	}
	return verifySignature(hashFile, signature)
}

func verifySignature(hashFile string, signature []byte) (bool, error) {
	data, err := ioutil.ReadFile(hashFile)
	if err != nil {
		return false, err
	}

	return bytes.Equal(signature, data), nil
}

func calcChunkHash(filename string) ([][32]byte, error) {
	list, err := makeChunk(filename)
	if err != nil {
		return nil, err
	}
	num := len(list)

	fr, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer fr.Close()

	var wg sync.WaitGroup
	pool, err := ants.NewPoolWithFunc(runtime.NumCPU()*1000, func(index interface{}) {
		i := index.(int)
		data, _ := doCalcHash(fr, i)
		if len(data) != 0 {
			list[i] = data
		}
		wg.Done()
	})
	if err != nil {
		return nil, err
	}
	defer pool.Release()

	for i := 0; i < num; i++ {
		wg.Add(1)
		_ = pool.Invoke(i)
	}
	wg.Wait()

	return list, nil
}

func doCalcHash(fr *os.File, idx int) ([32]byte, error) {
	var buf = make([]byte, defaultChunkSize)
	_, err := fr.ReadAt(buf, int64(defaultChunkSize*idx))
	if err != nil && err != io.EOF {
		return [32]byte{}, err
	}

	return sha256.Sum256(buf), nil
}

func makeChunk(filename string) ([][32]byte, error) {
	info, err := os.Stat(filename)
	if err != nil {
		return nil, err
	}

	total := info.Size()
	num := total / defaultChunkSize
	if total%defaultChunkSize != 0 {
		num += 1
	}

	var list = make([][32]byte, num)
	return list, nil
}
