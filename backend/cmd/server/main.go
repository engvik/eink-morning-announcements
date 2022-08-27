package main

import (
	"context"
	"log"

	"github.com/kelseyhightower/envconfig"

	"github.com/engvik/eink/backend/internal/config"
	"github.com/engvik/eink/backend/internal/server"
	"github.com/engvik/eink/backend/internal/tasks"
	"github.com/engvik/eink/backend/internal/transport"
	"github.com/engvik/eink/backend/pkg/calendar"
)

func main() {
	var cfg config.Config

	if err := envconfig.Process("eink-backend", &cfg); err != nil {
		log.Fatal("Error processing env vars: %s", err)
	}

	ctx := context.Background()

	httpClient := transport.NewHTTPClient()

	calendarParser := calendar.NewParser()
	calendar := calendar.NewTask(httpClient, calendarParser, &cfg)

	tasks.Start(calendar)

	s := server.New(&cfg)
	s.Serve(ctx)
}
