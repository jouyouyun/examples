package main

import (
	"flag"
	"github.com/google/go-cloud/blob"
	"github.com/myml/cloud-blob/ucloud"
	"log"
)

var (
	privateKey = flag.String("priv", "", "the bucket private key")
	publicKey  = flag.String("pub", "", "the bucket public key")
	bucketName = flag.String("b", "", "the bucket")
	key        = flag.String("P", "", "the ucloud key")
	action     = flag.String("m", "GET", "the action, contains: GET, DELETE, LIST")

	bucket *blob.Bucket
)

func main() {
	flag.Parse()

	var auth = ucloudBlob.MakeAuth(*publicKey, *privateKey)
	bucket = blob.NewBucket(ucloudBlob.OpenUcloudBucket(*bucketName, auth))

	var data []byte
	var err error
	switch *action {
	case "GET":
		data, err = Get(*key)
	case "DELETE":
		err = Delete(*key)
	case "LIST":
		_, err = List(*key)
	default:
		log.Fatal("Invalid action")
		flag.Usage()
		return
	}
	if err != nil {
		log.Fatal("Failed to get:", err)
		return
	}
	if len(data) != 0 {
		log.Print("[Get] result:", string(data))
	}
}
