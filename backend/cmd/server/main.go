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
	"github.com/engvik/eink-morning-announcements/backend/pkg/news"
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
	calendarFetcher := calendar.NewFetcher(&cfg, httpClient)
	calendarParser := calendar.NewParser(&cfg)
	calendarTask := calendar.NewTask(&cfg, storage, calendarFetcher, calendarParser)
	calendarHandlers := calendar.NewHTTPHandlers(&cfg, storage)

	// Weather
	weatherFetcher, err := weather.NewFetcher(&cfg, httpClient)
	if err != nil {
		log.Fatal("Error creating weather fetcher:", err)
	}

	weatherTask := weather.NewTask(&cfg, weatherFetcher, storage)
	weatherHandlers := weather.NewHTTPHandlers(&cfg, storage)

	// News
	newsFetcher := news.NewFetcher(httpClient)
	newsParser := news.NewParser(&cfg)
	newsTask := news.NewTask(&cfg, storage, newsFetcher, newsParser)
	newsHandlers := news.NewHTTPHandlers(storage)

	// Message
	messageHandlers := message.NewHTTPHandlers(storage)

	// Meta
	metaHandlers := meta.NewHTTPHandlers(&cfg, storage)

	// Create HTTP server
	s := server.New(&cfg)

	// Mount routes
	s.Mount("/api", calendarHandlers.Routes())
	s.Mount("/api", weatherHandlers.Routes())
	s.Mount("/api", messageHandlers.Routes())
	s.Mount("/api", metaHandlers.Routes())
	s.Mount("/api", newsHandlers.Routes())

	// Start background tasks
	tasks.Start(ctx, calendarTask, weatherTask, newsTask)

	// Start HTTP server
	s.Serve(ctx)
}
