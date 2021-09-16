package main

import (
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
	pkg    = flag.String("pkg", "", "the package name")
	file   = flag.String("file", "", "the file path")
	prefix = flag.String("prefix", "", "the file path prefix in verify")
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
	if ((len(*dir) == 0 || len(*pkg) == 0) || len(*file) == 0) && len(*output) == 0 {
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

	verified, err := verifyFile(*file, *prefix, *output)
	if err != nil {
		fmt.Println("Failed to verify:", err)
		os.Exit(-11)
	}
	fmt.Println("Verify result:", verified)
}

func signFile(srcFile, fpath, hashDir string) error {
	fmt.Println("Will signature:", srcFile, fpath)
	signature, err := signer.SignFileByChunk(srcFile)
	if err != nil {
		return err
	}
	return writeSignature(signature, fpath, hashDir)
}

func verifyFile(fpath, prefixDir string, hashDir string) (bool, error) {
	hashFile := filepath.Join(*output, fmt.Sprintf("%x",
		sha256.Sum256([]byte(filepath.Join("/", strings.TrimPrefix(fpath, prefixDir))))))

	return signer.VerifyFileByChunk(fpath, hashFile+".sign")
}

func walkDir(fpath string, info fs.FileInfo, err error) error {
	if !info.Mode().IsRegular() {
		return nil
	}

	if (info.Mode().Perm()&0111 == 0) && !filetype.IsExecutableFile(fpath) {
		return nil
	}
	return signFile(fpath,
		filepath.Join("/", correctFilepath(fpath, *dir, *pkg)), *output)
}

func writeSignature(signature []byte, fpath, hashDir string) error {
	_ = os.MkdirAll(*output, 0755)
	name := fmt.Sprintf("%x", sha256.Sum256([]byte(fpath)))
	fmt.Println("\tFile hash name:", name)
	return ioutil.WriteFile(filepath.Join(hashDir, name+".sign"), signature, 0644)
}

func correctFilepath(srcFile, prefixDir, pkgName string) string {
	var fixList = []struct {
		prefix string
		target string
	}{{
		prefix: "/DEBIAN/",
		target: "/var/lib/dpkg/info",
	}}
	fpath := strings.TrimPrefix(srcFile, prefixDir)
	for i := 0; i < len(fixList); i++ {
		if strings.HasPrefix(fpath, fixList[i].prefix) {
			fpath = strings.Replace(fpath, fixList[i].prefix,
				filepath.Join(fixList[i].target, pkgName+"."), 1)
			break
		}
	}
	return fpath
}
