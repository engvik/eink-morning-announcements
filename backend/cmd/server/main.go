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
	"github.com/engvik/eink/backend/pkg/weather"
)

func main() {
	var cfg config.Config

	if err := envconfig.Process("eink-backend", &cfg); err != nil {
		log.Fatal("Error processing env vars:", err)
	}

	ctx := context.Background()

	httpClient := transport.NewHTTPClient(&cfg)

	sqlClient, err := storage.NewSQLiteClient(&cfg)
	if err != nil {
		log.Fatal("Error creating SQLite client:", err)
	}
	defer sqlClient.Close()

	storage := storage.New(sqlClient)

	// Calendar
	calendarFetcher := calendar.NewFetcher(httpClient, &cfg)
	calendarParser := calendar.NewParser(&cfg)
	calendarTask := calendar.NewTask(storage, calendarFetcher, calendarParser, &cfg)

	// Weather
	weatherFetcher := weather.NewFetcher(httpClient, &cfg)
	weatherTask := weather.NewTask(weatherFetcher, storage, &cfg)

	// Start background tasks
	tasks.Start(calendarTask, weatherTask)

	// Start HTTP server
	s := server.New(&cfg)
	s.Serve(ctx)
}
