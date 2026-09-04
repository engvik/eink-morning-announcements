package main

import (
	"context"
	"log"

	"github.com/kelseyhightower/envconfig"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/server"
	"github.com/engvik/eink-morning-announcements/backend/internal/transport"
	"github.com/engvik/eink-morning-announcements/backend/pkg/calendar"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
	"github.com/engvik/eink-morning-announcements/backend/pkg/meta"
	"github.com/engvik/eink-morning-announcements/backend/pkg/storage"
	"github.com/engvik/eink-morning-announcements/backend/pkg/tasks"
	"github.com/engvik/eink-morning-announcements/backend/pkg/weather"
)

func main() {
	var cfg config.Config

	if err := envconfig.Process("eink-morning-announcements-backend", &cfg); err != nil {
		log.Fatal("Error processing env vars:", err)
	}

	ctx := context.Background()

	httpClient := transport.NewHTTPClient(&cfg)

	storageClient, err := storage.NewClient(&cfg)
	if err != nil {
		log.Fatal("Error creating storage client:", err)
	}
	defer storageClient.Close()

	storage := storage.New(storageClient)

	// Calendar
	calendarFetcher := calendar.NewFetcher(httpClient, &cfg)
	calendarParser := calendar.NewParser(&cfg)
	calendarTask := calendar.NewTask(&cfg, storage, calendarFetcher, calendarParser)
	calendarHandler := calendar.NewHTTPHandler(&cfg, storage)

	// Weather
	weatherFetcher := weather.NewFetcher(&cfg, httpClient)
	weatherTask := weather.NewTask(&cfg, weatherFetcher, storage)
	weatherHandler := weather.NewHTTPHandler(&cfg, storage)

	// Message
	messageHandler := message.NewHTTPHandler(storage)

	// Meta
	metaHandler := meta.NewHTTPHandler(&cfg)

	// Create HTTP server
	s := server.New(&cfg)

	// Mount routes
	s.MountRoute("/api/calendar", calendarHandler)
	s.MountRoute("/api/weather", weatherHandler)
	s.MountRoute("/api/message", messageHandler)
	s.MountRoute("/api/meta", metaHandler)

	// Start background tasks
	tasks.Start(ctx, calendarTask, weatherTask)

	// Start HTTP server
	s.Serve(ctx)
}
