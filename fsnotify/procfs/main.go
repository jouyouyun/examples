package main

import (
	"fmt"
	"os"

	"github.com/fsnotify/fsnotify"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: <%s> <filepath>\n", os.Args[0])
		return
	}

	watcher, err := fsnotify.NewWatcher()
	if err != nil {
		fmt.Println("Failed to new watcher:", err)
		return
	}
	defer watcher.Close()

	err = watcher.Add(os.Args[1])
	if err != nil {
		fmt.Println("Failed to watch pid:", err)
		return
	}

	quit := false
	for {
		if quit {
			break
		}

		select {
		case ev := <-watcher.Events:
			fmt.Printf("Event: %v\n", ev)
		case e := <-watcher.Errors:
			fmt.Println("Error occurred:", e)
			break
		}
	}
}
