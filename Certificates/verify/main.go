package main

import (
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
)

const (
	SubjectNameCertificate = "certificate"
	SubjectNameSignature   = "signature"
)

// SignHeaderSubject store every header subject name, offset and length
type SignHeaderSubject struct {
	Name   string
	Offset uint64
	Length uint64
}

// SignHeaderSubjectList store header subject list
type SignHeaderSubjectList []*SignHeaderSubject

// SignFileHeader store signature file header
type SignFileHeader struct {
	Subjects SignHeaderSubjectList
}

// SignFileInfo store file content after decode
type SignFileInfo struct {
	Header  *SignFileHeader
	Content []byte
}

func verifyCertificateChain(parentCer, certCer []byte) (bool, error) {
	parentCert, err := x509.ParseCertificate(parentCer)
	if err != nil {
		return false, err
	}
	fmt.Println("Root algorithm:", parentCert.SignatureAlgorithm, parentCert.Subject.String())

	cert, err := x509.ParseCertificate(certCer)
	if err != nil {
		return false, err
	}
	fmt.Println("Target algorithm:", cert.SignatureAlgorithm, cert.Subject.String())

	err = cert.CheckSignatureFrom(parentCert)
	if err != nil {
		return false, err
	}
	return true, nil
}

func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <parent cert> <target cert> <target sign>\n", os.Args[0])
		return
	}

	parentCer, err := ioutil.ReadFile(os.Args[1])
	if err != nil {
		fmt.Println("Failed to read parent cert:", err)
		return
	}

	certCer, err := ioutil.ReadFile(os.Args[2])
	if err != nil {
		fmt.Println("Failed to read target cert:", err)
		return
	}

	_, err = verifyCertificateChain(parentCer, certCer)
	if err != nil {
		fmt.Println("Failed to verify:", err)
		return
	}
	fmt.Println("Verify success!")
}
