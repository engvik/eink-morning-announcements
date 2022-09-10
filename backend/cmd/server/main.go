package main

import (
	"context"
	"log"

	"github.com/kelseyhightower/envconfig"

	"github.com/engvik/eink/backend/internal/config"
	"github.com/engvik/eink/backend/internal/server"
	"github.com/engvik/eink/backend/internal/transport"
	"github.com/engvik/eink/backend/pkg/calendar"
	"github.com/engvik/eink/backend/pkg/storage"
	"github.com/engvik/eink/backend/pkg/tasks"
)

func main() {
	var cfg config.Config

	if err := envconfig.Process("eink-backend", &cfg); err != nil {
		log.Fatal("Error processing env vars:", err)
	}

	ctx := context.Background()

	httpClient := transport.NewHTTPClient()

	sqlClient, err := storage.NewSQLiteClient(&cfg)
	if err != nil {
		log.Fatal("Error creating SQLite client:", err)
	}
	defer sqlClient.Close()

	storage := storage.New(sqlClient)

	calendarParser := calendar.NewParser(&cfg)
	calendar := calendar.NewTask(httpClient, storage, calendarParser, &cfg)

	tasks.Start(calendar)

	s := server.New(&cfg)
	s.Serve(ctx)
}
