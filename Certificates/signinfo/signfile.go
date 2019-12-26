package signinfo

import (
	"bytes"
	"crypto"
	"crypto/rsa"
	"crypto/sha256"
	"crypto/x509"
	"encoding/gob"
	"fmt"
)

const (
	subjectNameCertificate = "certificate"
	subjectNameSignature   = "signature"

	errInvalidArgs        = "invalid arguments"
	errInvalidHeader      = "invalid header"
	errNoSignature        = "no signature found"
	errNoCertificate      = "no certificate found"
	errNoPublicKey        = "no public key found"
	errUnsupportAlgorithm = "unsupported crypto algorithm"
)

// HeaderSubject store every header subject name, offset and length
type HeaderSubject struct {
	Name   string
	Offset int
	Size   int
}

// HeaderSubjectList store header subject list
type HeaderSubjectList []*HeaderSubject

// SignInfoHeader store signature file header
type SignInfoHeader struct {
	Subjects HeaderSubjectList
}

// SignInfo store file content after decode
type SignInfo struct {
	Header *SignInfoHeader
	Body   []byte
}

// Get return the header subject by name
func (subjects HeaderSubjectList) Get(name string) *HeaderSubject {
	for _, v := range subjects {
		if v.Name == name {
			return v
		}
	}
	return nil
}

// GenSignInfo generate SignInfo. The signData is the signatured data,
// the certData is the certificate which associated with the private key
func GenSignInfo(signedData, certData []byte) ([]byte, error) {
	if len(signedData) == 0 || len(certData) == 0 {
		return nil, fmt.Errorf(errInvalidArgs)
	}

	var subjects = HeaderSubjectList{
		&HeaderSubject{
			Name:   subjectNameSignature,
			Offset: 0,
			Size:   len(signedData),
		},
		&HeaderSubject{
			Name:   subjectNameCertificate,
			Offset: len(signedData),
			Size:   len(certData),
		},
	}

	signedData = append(signedData, certData...)
	var info = SignInfo{
		Header: &SignInfoHeader{Subjects: subjects},
		Body:   signedData,
	}

	return gobEncode(&info)
}

// VerifySignInfo verify sign info validity
// The parentCerList is the signInfo's certificate's parent certificate list.
// The fcontents is the signInfo data.
// The signature is the original data.
func VerifySignInfo(parentCerList [][]byte, fcontents, signature []byte) error {
	if len(fcontents) == 0 || len(signature) == 0 || len(parentCerList) == 0 {
		return fmt.Errorf(errInvalidArgs)
	}

	var info SignInfo
	err := gob.NewDecoder(bytes.NewReader(fcontents)).Decode(&info)
	if err != nil {
		return err
	}

	if info.Header == nil || len(info.Header.Subjects) == 0 {
		return fmt.Errorf(errInvalidHeader)
	}

	certSubject := info.Header.Subjects.Get(subjectNameCertificate)
	if certSubject == nil {
		return fmt.Errorf(errNoCertificate)
	}

	signSubject := info.Header.Subjects.Get(subjectNameSignature)
	if signSubject == nil {
		return fmt.Errorf(errNoSignature)
	}

	certCer := info.Body[certSubject.Offset : certSubject.Offset+certSubject.Size]
	signed := info.Body[signSubject.Offset : signSubject.Offset+signSubject.Size]
	// check certificate validity
	return checkCertificateSignature(parentCerList, certCer, signed, signature)
}

func gobEncode(info *SignInfo) ([]byte, error) {
	var buf bytes.Buffer
	var encoder = gob.NewEncoder(&buf)

	err := encoder.Encode(info)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func checkCertificateSignature(parentCerList [][]byte, certCer, signed, signature []byte) error {
	cert, err := x509.ParseCertificate(certCer)
	if err != nil {
		return err
	}

	err = verifySignature(cert, signed, signature)
	if err != nil {
		return err
	}

	for _, cer := range parentCerList {
		parent, err1 := x509.ParseCertificate(cer)
		if err1 != nil {
			err = err1
			continue
		}

		err = cert.CheckSignatureFrom(parent)
		if err != nil {
			continue
		}

		return nil
	}

	return err
}

func verifySignature(cert *x509.Certificate, signed, signature []byte) error {
	switch cert.SignatureAlgorithm {
	case x509.SHA256WithRSA:
		pubKey, ok := cert.PublicKey.(*rsa.PublicKey)
		if !ok {
			return fmt.Errorf(errNoPublicKey)
		}

		var h = sha256.New()
		h.Write(signature)
		var d = h.Sum(nil)
		return rsa.VerifyPKCS1v15(pubKey, crypto.SHA256, d, signed)
	}

	// TODO(jouyouyun): support more encryption algorithm
	return fmt.Errorf(errUnsupportAlgorithm)
}
