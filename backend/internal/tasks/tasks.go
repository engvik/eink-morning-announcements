package tasks

import "log"

type Task interface {
	Name() string
	Run()
}

func Start(tasks ...Task) {
	for _, t := range tasks {
		log.Println("Starting task:", t.Name())
		go t.Run()
	}
}
