package main

import (
	"fmt"

	. "../adapter"
)

type Dell7590 struct{}

func (info *Dell7590) Apply() error {
	fmt.Println("Apply dell 7590")
	return nil
}

func (info *Dell7590) Check() error {
	fmt.Println("Check dell 7590")
	return nil
}

func NewObject() Adapter {
	return &Dell7590{}
}

var Object Adapter

func init() {
	Object = &Dell7590{}
}
