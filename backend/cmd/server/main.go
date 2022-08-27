package main

import (
	"context"
	"log"

	"github.com/kelseyhightower/envconfig"

	"github.com/engvik/eink/backend/internal/config"
	"github.com/engvik/eink/backend/internal/server"
)

func main() {
	var cfg config.Config

	if err := envconfig.Process("eink-backend", &cfg); err != nil {
		log.Fatal("Error processing env vars: %s", err)
	}

	ctx := context.Background()

	s := server.New(&cfg)
	s.Serve(ctx)
}
