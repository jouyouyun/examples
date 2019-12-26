package signinfo

import (
	"crypto"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha256"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"io/ioutil"
	"testing"
)

func TestSignInfo(t *testing.T) {
	var data = []byte("hello, world")
	signed, err := signature(data)
	if err != nil {
		t.Error("failed to signature:", err)
		return
	}

	certData, err := loadCertificate("testdata/1.crt")
	if err != nil {
		t.Error("failed to load certificate:", err)
		return
	}

	signInfo, err := GenSignInfo(signed, certData)
	if err != nil {
		t.Error("failed to generate signature info:", err)
		return
	}
	ioutil.WriteFile("/tmp/sign.info", signInfo, 0644)

	parentData, err := loadCertificate("testdata/intermediate.crt")
	if err != nil {
		t.Error("failed to load parent certificate:", err)
		return
	}

	err = VerifySignInfo([][]byte{parentData}, signInfo, []byte(data))
	if err != nil {
		t.Error("failed to verify signature:", err)
	}
}

func signature(data []byte) ([]byte, error) {
	contents, err := ioutil.ReadFile("testdata/1.key")
	if err != nil {
		return nil, err
	}

	block, _ := pem.Decode(contents)
	if block == nil {
		return nil, fmt.Errorf("failed to pem decode private key")
	}

	privKey, err := x509.ParsePKCS8PrivateKey(block.Bytes)
	if err != nil {
		return nil, err
	}

	var h = sha256.New()
	h.Write(data)
	var d = h.Sum(nil)
	return rsa.SignPKCS1v15(rand.Reader, privKey.(*rsa.PrivateKey), crypto.SHA256, d)
}

func loadCertificate(filename string) ([]byte, error) {
	certData, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	block, _ := pem.Decode(certData)
	if block == nil {
		return nil, fmt.Errorf("failed to pem decode certificate")
	}

	return block.Bytes, nil
}
