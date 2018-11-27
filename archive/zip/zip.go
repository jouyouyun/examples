package main

import (
	"archive/zip"
	"bytes"
	"compress/flate"
	"flag"
	"io"
	"io/ioutil"
	"log"
	"os"
)

var (
	method = flag.String("m", "r", "the zip method: r/w, default: r")
)

func main() {
	flag.Parse()
	if len(os.Args) < 4 {
		flag.Usage()
		return
	}

	var data []byte
	var err error
	switch *method {
	case "r":
		err = zipDecompress(os.Args[3])
	case "w":
		data, err = zipCompress(os.Args[3:])
		log.Print("[Compress] data len:", len(data))
	}
	if err != nil {
		log.Fatal("Failed for:", *method, os.Args[3:])
	}
	if len(data) != 0 {
		err = ioutil.WriteFile("/tmp/zip_file.zip", data, 0644)
		if err != nil {
			log.Fatal("Failed to write file:", err)
		}
	}
}

func zipCompress(files []string) ([]byte, error) {
	var buf = new(bytes.Buffer)
	var writer = zip.NewWriter(buf)
	writer.RegisterCompressor(zip.Deflate,
		func(out io.Writer) (io.WriteCloser, error) {
			return flate.NewWriter(out, flate.BestCompression)
		})
	for _, f := range files {
		log.Print("[Compress] file:", f)
		fw, err := writer.Create(f)
		if err != nil {
			writer.Close()
			return nil, err
		}
		contents, err := ioutil.ReadFile(f)
		if err != nil {
			writer.Close()
			return nil, err
		}
		_, err = fw.Write(contents)
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

func zipDecompress(filename string) error {
	reader, err := zip.OpenReader(filename)
	if err != nil {
		return err
	}
	defer reader.Close()

	reader.RegisterDecompressor(zip.Deflate,
		func(in io.Reader) io.ReadCloser {
			return flate.NewReader(in)
		})

	for _, f := range reader.File {
		log.Print("[Decompress] filename:", f.Name)
		fr, err := f.Open()
		if err != nil {
			return err
		}
		contents, err := ioutil.ReadAll(fr)
		if err != nil {
			return err
		}
		err = ioutil.WriteFile(f.Name, contents, 0644)
		if err != nil {
			return err
		}
	}
	return nil
}
