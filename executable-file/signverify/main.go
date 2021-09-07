package main

import (
	"bytes"
	"crypto/sha256"
	"flag"
	"fmt"
	"io/fs"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"./filetype"
	"./signer"
)

var (
	action = flag.String("action", "", "avaliable action: sign、verify")
	dir    = flag.String("dir", "", "the directory path")
	file   = flag.String("file", "", "the file path")
	output = flag.String("output", "", "the output directory")
)

func main() {
	flag.Parse()

	switch *action {
	case "sign":
		goto sign
	case "verify":
		goto verify
	default:
		flag.Usage()
		os.Exit(-1)
	}

sign:
	if (len(*dir) == 0 || len(*file) == 0) && len(*output) == 0 {
		flag.Usage()
		os.Exit(-1)
	}
	if len(*dir) != 0 {
		err := filepath.Walk(*dir, walkDir)
		if err != nil {
			fmt.Println("Failed to walk dir:", err)
			os.Exit(-2)
		}
	}

	if len(*file) != 0 {
		err := signFile(*file, *file, *output)
		if err != nil {
			fmt.Println("Failed to sum file signature:", err)
			os.Exit(-3)
		}
	}
	os.Exit(0)

verify:
	if len(*file) == 0 && len(*output) == 0 {
		flag.Usage()
		os.Exit(-1)
	}

	verified, err := verifyFile(*file, *output)
	if err != nil {
		fmt.Println("Failed to verify:", err)
		os.Exit(-11)
	}
	fmt.Println("Verify result:", verified)
}

func signFile(srcFile, fpath, hashDir string) error {
	fmt.Println("Will signature:", srcFile, fpath)
	signature, err := signer.SignFileWithFragment(srcFile)
	if err != nil {
		return err
	}
	return writeSignature(signature, fpath, hashDir)
}

func verifyFile(fpath string, hashDir string) (bool, error) {
	signature, err := signer.SignFileWithFragment(fpath)
	if err != nil {
		return false, err
	}

	hashFile := filepath.Join(*output, fmt.Sprintf("%x", sha256.Sum256([]byte(fpath))))
	data, err := ioutil.ReadFile(hashFile)
	if err != nil {
		return false, err
	}

	return bytes.Equal(signature, data), nil
}

func walkDir(fpath string, info fs.FileInfo, err error) error {
	if !info.Mode().Perm().IsRegular() {
		return nil
	}

	if !filetype.IsExecutableFile(fpath) {
		return nil
	}
	return signFile(fpath,
		filepath.Join("/", strings.TrimLeft(fpath, *dir)), *output)
}

func writeSignature(signature []byte, fpath, hashDir string) error {
	_ = os.MkdirAll(*output, 0755)
	name := fmt.Sprintf("%x", sha256.Sum256([]byte(fpath)))
	fmt.Println("\tFile hash name:", name)
	return ioutil.WriteFile(filepath.Join(hashDir, name), signature, 0644)
}
