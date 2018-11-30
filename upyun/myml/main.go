package main

import (
	"flag"
	"github.com/google/go-cloud/blob"
	"github.com/myml/cloud-blob/upyun"
	"log"
)

var (
	operator   = flag.String("o", "", "the bucket operator")
	passwd     = flag.String("p", "", "the operator password")
	bucketName = flag.String("b", "", "the bucket")
	key        = flag.String("k", "", "the upyun key")
	action     = flag.String("m", "GET", "the action, contains: GET, DELETE, LIST, SIZE")

	bucket *blob.Bucket
)

func main() {
	flag.Parse()

	var auth = upyunBlob.MakeAuth(*operator, *passwd)
	bucket = blob.NewBucket(upyunBlob.OpenUpyunBucket(*bucketName, auth))

	var data []byte
	var size uint64
	var err error
	switch *action {
	case "GET":
		data, err = Get(*key)
	case "DELETE":
		err = Delete(*key)
	case "LIST":
		_, err = List(*key)
	case "SIZE":
		size, err = Size(*key)
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
	if size != 0 {
		log.Print("[Size] result(byte):", size)
	}
}
