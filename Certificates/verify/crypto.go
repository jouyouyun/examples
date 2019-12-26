package main

// CryptoHandler handle crypto, descrypt, signature and verify etc.
type CryptoHandler interface {
	Signature(data []byte) ([]byte, error)
	GenSignFile(data []byte, certFile, outFile string) error
}
