package server

import (
	"context"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	"github.com/go-chi/chi"

	"github.com/engvik/eink/backend/internal/config"
)

type Server struct {
	Port   string
	Router *chi.Mux
	HTTP   *http.Server
}

func New(cfg *config.Config) *Server {
	router := chi.NewRouter()

	return &Server{
		Port:   cfg.Port,
		Router: router,
		HTTP: &http.Server{
			Addr:    fmt.Sprintf(":%s", cfg.Port),
			Handler: router,
		},
	}
}

func (s *Server) Serve(ctx context.Context) {
	go func(ctx context.Context, server *http.Server) {
		stop := make(chan os.Signal, 1)
		signal.Notify(stop, os.Interrupt, syscall.SIGTERM)

		<-stop

		log.Println("Shutdown signal received")

		if err := server.Shutdown(ctx); err != nil {
			log.Fatalf("Error gracefully shutting down server: %s", err)
		}
	}(ctx, s.HTTP)

	log.Printf("Listening at: %s\n", s.Port)

	if err := s.HTTP.ListenAndServe(); err != http.ErrServerClosed {
		log.Fatalf("Error serving: %s", err)
	}
}
