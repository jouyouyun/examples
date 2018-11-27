package main

import (
	"bytes"
	"compress/gzip"
	"flag"
	"io/ioutil"
	"log"
	"os"
)

var (
	method = flag.String("m", "r", "the method: r/w, default: r")
)

func main() {
	flag.Parse()
	if len(os.Args) < 4 {
		flag.Usage()
	}

	var (
		data []byte
		err  error
	)
	switch *method {
	case "r":
		err = gzipDecompress(os.Args[3])
	case "w":
		data, err = gzipCompress(os.Args[3:])
	}
	if err != nil {
		log.Fatal("Failed for:", *method, err)
	}
	if len(data) != 0 {
		err = ioutil.WriteFile("/tmp/gzip_file.zip", data, 0644)
		if err != nil {
			log.Fatal("Failed to write:", err)
		}
	}
}

func gzipCompress(files []string) ([]byte, error) {
	var buf = new(bytes.Buffer)
	var writer = gzip.NewWriter(buf)
	for _, f := range files {
		contents, err := ioutil.ReadFile(f)
		if err != nil {
			writer.Close()
			return nil, err
		}
		_, err = writer.Write(contents)
		if err != nil {
			writer.Close()
			return nil, err
		}
	}

	err := writer.Close()
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func gzipDecompress(filename string) error {
	return nil
}
