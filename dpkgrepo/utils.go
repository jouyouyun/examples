package dpkgrepo

import (
	"compress/gzip"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
)

func Download(url string) (*http.Response, error) {
	resp, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	if resp == nil {
		return nil, ErrHTTPNoResp
	}
	if resp.StatusCode != http.StatusOK {
		return nil, ErrHTTPStatus
	}
	return resp, nil
}

func DownloadToFile(url, filename string) error {
	err := os.MkdirAll(filepath.Dir(filename), 0755)
	if err != nil {
		return err
	}
	fp, err := os.OpenFile(filename, os.O_WRONLY|os.O_CREATE, 0644)
	if err != nil {
		return err
	}
	defer fp.Close()

	resp, err := Download(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	_, err = io.Copy(fp, resp.Body)
	return err
}

func GZipDecompressToFile(src, dest string) error {
	fr, err := os.Open(src)
	if err != nil {
		return err
	}
	defer fr.Close()

	err = os.MkdirAll(filepath.Dir(dest), 0755)
	if err != nil {
		return err
	}

	reader, err := gzip.NewReader(fr)
	if err != nil {
		return err
	}
	defer reader.Close()
	data, err := ioutil.ReadAll(reader)
	if err != nil {
		return err
	}
	return ioutil.WriteFile(dest, data, 0644)
}
