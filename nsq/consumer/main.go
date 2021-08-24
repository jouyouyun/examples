package main

import (
	"fmt"
	"time"

	gnsq "github.com/nsqio/go-nsq"
)

func handlerMessage(msg *gnsq.Message) error {
	fmt.Println("Received message:", string(msg.Body))
	time.Sleep(time.Second * 65)
	fmt.Println("Handler done")
	return nil
}

func main() {
	consumer, err := gnsq.NewConsumer("timeout", "ch", gnsq.NewConfig())
	if err != nil {
		fmt.Println("failed to new consumer:", err)
		return
	}

	consumer.AddHandler(gnsq.HandlerFunc(handlerMessage))
	err = consumer.ConnectToNSQD("127.0.0.1:8150")
	if err != nil {
		fmt.Println("failed to connect nsqd:", err)
	}

	var quit = make(chan struct{}, 1)
	<-quit
	fmt.Println("Will quit")
	consumer.Stop()
}
