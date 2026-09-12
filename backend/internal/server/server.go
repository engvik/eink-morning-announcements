package server

import (
	"context"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/transport"
)

type Server struct {
	port   string
	router *http.ServeMux
	auth   func(http.Handler) http.Handler
	server *http.Server
}

// Route is a handler at Path relative to the prefix it is mounted under.
type Route struct {
	Method  string
	Path    string
	Handler http.HandlerFunc
	Public  bool
}

func New(cfg *config.Config) *Server {
	router := http.NewServeMux()

	s := &Server{
		port:   cfg.Port,
		router: router,
		server: &http.Server{
			Addr:    fmt.Sprintf(":%s", cfg.Port),
			Handler: transport.CORSHandler()(router),
		},
	}

	if cfg.Authorization != "" {
		s.auth = transport.NewAuthMiddleware(cfg.Authorization)
	}

	return s
}

func (s *Server) Mount(prefix string, routes []Route) {
	for _, r := range routes {
		var h http.Handler = r.Handler
		if !r.Public && s.auth != nil {
			h = s.auth(h)
		}

		s.router.Handle(r.Method+" "+prefix+r.Path, h)
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
	}(ctx, s.server)

	log.Printf("Listening at: %s\n", s.port)

	if err := s.server.ListenAndServe(); err != http.ErrServerClosed {
		log.Fatalf("Error serving: %s", err)
	}
}
