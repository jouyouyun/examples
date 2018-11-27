package main

import (
	"bytes"
	"compress/gzip"
	"flag"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
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
	var buf bytes.Buffer
	writer, err := gzip.NewWriterLevel(&buf, gzip.BestCompression)
	if err != nil {
		return nil, err
	}
	for _, f := range files {
		contents, err := ioutil.ReadFile(f)
		if err != nil {
			writer.Close()
			return nil, err
		}
		writer.Name = f
		_, err = writer.Write(contents)
		if err != nil {
			writer.Close()
			return nil, err
		}
		err = writer.Close()
		if err != nil {
			return nil, err
		}
		writer.Reset(&buf)
	}

	return buf.Bytes(), nil
}

func gzipDecompress(filename string) error {
	log.Print("[Decompress] filename:", filename)
	contents, err := ioutil.ReadFile(filename)
	if err != nil {
		return err
	}
	var buf = bytes.NewBuffer(contents)
	reader, err := gzip.NewReader(buf)
	if err != nil {
		return err
	}
	defer reader.Close()
	dir := "/tmp/decompress"
	err = os.MkdirAll(dir, 0755)
	if err != nil {
		return err
	}
	for {
		reader.Multistream(false)
		log.Print("[Decompress] file:", reader.Name)
		contents, err := ioutil.ReadAll(reader)
		if err != nil {
			log.Print("[Compress] Failed to read:", err)
			return err
		}
		fpath := filepath.Join(dir, filepath.Base(reader.Name))
		err = ioutil.WriteFile(fpath, contents, 0644)
		if err != nil {
			return err
		}
		err = reader.Reset(buf)
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
	}
	return nil
}
