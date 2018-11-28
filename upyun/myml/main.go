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
	key        = flag.String("P", "", "the upyun key")
	action     = flag.String("m", "GET", "the action, contains: GET, DELETE, LIST")

	bucket *blob.Bucket
)

func main() {
	flag.Parse()

	var auth = upyunBlob.MakeAuth(*operator, *passwd)
	bucket = blob.NewBucket(upyunBlob.OpenUpyunBucket(*bucketName, auth))

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
