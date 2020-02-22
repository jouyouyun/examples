package main

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
)

func example2(cmdName string) {
	cmd := exec.Command("/bin/sh", "-c", cmdName)
	stdout, _ := cmd.StdoutPipe()
	err := cmd.Start()
	if err != nil {
		fmt.Println("Failed to start cmd:", err)
		return
	}

	oneByte := make([]byte, 5120)
	for {
		_, err := stdout.Read(oneByte)
		if err != nil {
			fmt.Printf(err.Error())
			break
		}
		r := bufio.NewReader(stdout)
		line, _, _ := r.ReadLine()
		fmt.Println("[Output]:", string(line))
	}

	err = cmd.Wait()
	if err != nil {
		fmt.Println("Failed to wait cmd:", err)
		return
	}
}

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <command>\n", os.Args[0])
		return
	}
	example2(os.Args[1])
}
