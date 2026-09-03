package tasks

import (
	"context"
	"log"
)

type Task interface {
	Name() string
	Run(context.Context)
}

func Start(ctx context.Context, tasks ...Task) {
	for _, t := range tasks {
		log.Println("Starting task:", t.Name())
		go t.Run(ctx)
	}
}
