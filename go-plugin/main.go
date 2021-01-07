package main

import "./templates"

import "fmt"
import "plugin"
import "reflect"

func loadPlugin(str string) {
	plg, err := plugin.Open("/tmp/plugins/libtemplate_user.so")
	if err != nil {
		fmt.Println("Failed to open:", err)
		return
	}

	obj, err := plg.Lookup("Obj")
	if err != nil {
		fmt.Println("Failed to lookup:", err)
		return
	}

	re := reflect.TypeOf(obj)
	fmt.Println("Reflect:", re.String())
	temp, ok := obj.(*templates.Templates)
	if !ok {
		fmt.Println("Invalid type")
		return
	}

	_, _ = (*temp).Start([]byte(str))
}

func main() {
	fmt.Println("vim-go")

	loadPlugin("Hello")
	loadPlugin("123456")
}
