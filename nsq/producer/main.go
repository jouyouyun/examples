package main

import (
	"fmt"
	"math/rand"
	"time"

	gnsq "github.com/nsqio/go-nsq"
)

func genData() []byte {
	str := "1234567890qwertyuiopzxcvbnmasdfghjkl"
	slen := len(str)
	rand.Seed(time.Now().Unix() + 1000)
	idx := rand.Intn(slen)
	var data []byte
	for ; idx < slen; idx++ {
		data = append(data, str[idx])
	}
	return data
}

func main() {
	producer, err := gnsq.NewProducer("127.0.0.1:8150", gnsq.NewConfig())
	if err != nil {
		fmt.Println("failed to new producer:", err)
		return
	}

	for {
		err = producer.Publish("timeout", genData())
		if err != nil {
			fmt.Println("failed to publish:", err)
			break
		}
		time.Sleep(time.Second * 2)
	}

	producer.Stop()
}
