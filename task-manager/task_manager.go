package main

import (
	"fmt"
)

type taskManager struct {
	tasks map[string]*Task
}

func (manager *taskManager) Init() {
	// load the last unfinished tasks
}

func (manager *taskManager) createTask(task *Task) error {
	if _, ok := manager.tasks[task.Name]; ok {
		// has exists
		return fmt.Errorf("Task has exists")
	}
	go task.Start()
	return nil
}

func main() {
	var manager = taskManager{tasks: make(map[string]*Task)}
	manager.Init()

	// loop
}
