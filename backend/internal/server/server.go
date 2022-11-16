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

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/transport"
)

type Server struct {
	port   string
	router *chi.Mux
	server *http.Server
}

func New(cfg *config.Config) *Server {
	router := chi.NewRouter()
	router.Use(transport.CORSHandler())

	if cfg.Authorization != "" {
		router.Use(transport.NewAuthMiddleware(cfg.Authorization))
	}

	return &Server{
		port:   cfg.Port,
		router: router,
		server: &http.Server{
			Addr:    fmt.Sprintf(":%s", cfg.Port),
			Handler: router,
		},
	}
}

func (s *Server) MountRoute(path string, route http.Handler) {
	s.router.Mount(path, route)
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
	}(ctx, s.server)

	log.Printf("Listening at: %s\n", s.port)

	if err := s.server.ListenAndServe(); err != http.ErrServerClosed {
		log.Fatalf("Error serving: %s", err)
	}
}
