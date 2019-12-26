package main

import (
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"github.com/grantae/certinfo"
)

func parseCerFile(filename string) {
	data, err := ioutil.ReadFile(filename)
	if err != nil {
		fmt.Println("Failed to read file:", err)
		return
	}

	cert, err := x509.ParseCertificate(data)
	if err != nil {
		fmt.Println("Failed to parse data:", err)
		return
	}

	text, err := certinfo.CertificateText(cert)
	if err != nil {
		fmt.Println("Failed to parse certificate:", err)
		return
	}

	fmt.Println(text)
}

func parseCrtFile(filename string) {
	data, err := ioutil.ReadFile(filename)
	if err != nil {
		fmt.Println("Failed to read file:", err)
		return
	}

	block, _ := pem.Decode(data)
	if block == nil {
		fmt.Println("Failed to decode by pem")
		return
	}
	// fmt.Printf("Type: %s\n\n", block.Bytes)

	cert, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		cert, err = x509.ParseCertificate(data)
		if err != nil {
			fmt.Println("Failed to parse pem data:", err)
			return
		}
	}

	text, err := certinfo.CertificateText(cert)
	if err != nil {
		fmt.Println("Failed to parse certificate:", err)
		return
	}

	fmt.Println(text)
}

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <cer file>/<crt file>\n", os.Args[0])
		return
	}

	if strings.HasSuffix(os.Args[1], "cer") {
		parseCerFile(os.Args[1])
	} else if strings.HasSuffix(os.Args[1], "crt") {
		parseCrtFile(os.Args[1])
	}
}
