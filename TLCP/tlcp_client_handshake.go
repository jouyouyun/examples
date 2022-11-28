package main

import (
	"flag"
	"fmt"
	"io/ioutil"

	"gitee.com/Trisia/gotlcp/tlcp"
	"github.com/emmansun/gmsm/smx509"
)

var (
	_ca   = flag.String("ca", "", "The CA certificate file")
	_cert = flag.String("cert", "", "The public key certificate file")
	_key  = flag.String("key", "", "The private key certificate file")
	_host = flag.String("host", "", "The server domain or ip")
	_port = flag.Int("port", 443, "The server port")
)

func main() {
	flag.Parse()

	if len(*_ca) == 0 || len(*_cert) == 0 || len(*_key) == 0 || len(*_host) == 0 {
		flag.Usage()
	}

	caData, err := ioutil.ReadFile(*_ca)
	if err != nil {
		fmt.Println("Read ca file failed:", err)
		return
	}
	certData, err := ioutil.ReadFile(*_cert)
	if err != nil {
		fmt.Println("Read cert file failed:", err)
		return
	}
	keyData, err := ioutil.ReadFile(*_key)
	if err != nil {
		fmt.Println("Read key file failed:", err)
		return
	}
	caCert, err := smx509.ParseCertificatePEM(caData)
	if err != nil {
		fmt.Println("Parse ca failed:", err)
	}
	authCert, err := tlcp.X509KeyPair(certData, keyData)
	if err != nil {
		fmt.Println("Parse key pair failed:", err)
	}

	caPool := smx509.NewCertPool()
	caPool.AddCert(caCert)

	conn, err := tlcp.Dial("tcp", fmt.Sprintf("%s:%d", *_host, *_port), &tlcp.Config{
		RootCAs:      caPool,
		Certificates: []tlcp.Certificate{authCert},
	})
	if err != nil {
		fmt.Println("Dial failed:", err)
		return
	}
	defer conn.Close()

	err = conn.Handshake()
	if err != nil {
		fmt.Println("Handshake failed:", err)
		return
	}

	fmt.Println("Handshake success!")
}
