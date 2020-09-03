package main

import (
	"fmt"
	"plugin"

	"reflect"

	"./adapter"
)

func testAdapter(adp adapter.Adapter) {
	fmt.Println("\tstart test adapter...")
	if err := adp.Apply(); err != nil {
		fmt.Println(err)
	}
	if err := adp.Check(); err != nil {
		fmt.Println(err)
	}
	fmt.Println("\ttest done")
}

func main() {
	fmt.Println("Start test...")

	plg, err := plugin.Open("./plugin/dell7590.so")
	if err != nil {
		fmt.Println("Failed to open plugin:", err)
		return
	}

	obj, err := plg.Lookup("Object")
	if err != nil {
		fmt.Println("Failed to lookup obj:", err)
		return
	}

	dell7590, ok := obj.(*adapter.Adapter)
	if !ok {
		fmt.Printf("Failed to convert to adapter from Object, %#v\n", reflect.TypeOf(obj).String())
	} else {
		testAdapter(*dell7590)
	}

	fobj, err := plg.Lookup("NewObject")
	if err != nil {
		fmt.Println("Failed to lookup NewObject:", err)
		return
	}

	ret := fobj.(func() adapter.Adapter)()
	if ret == nil {
		fmt.Println("Failed to convert to adapter from NewObject")
	} else {
		testAdapter(ret)
	}
	fmt.Println("Done")
}
