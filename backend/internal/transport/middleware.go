package transport

import (
	"net/http"
	"strings"

	"github.com/go-chi/cors"
)

func NewAuthMiddleware(auth string, allowUnauthenticated []string) func(next http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			endpoint := r.URL.String()

			for _, allowed := range allowUnauthenticated {
				if allowed == endpoint {
					next.ServeHTTP(w, r)
					return
				}
			}

			splitHeader := strings.Split(r.Header.Get("Authorization"), "Bearer")

			if len(splitHeader) != 2 {
				w.WriteHeader(http.StatusBadRequest)
				w.Write([]byte(http.StatusText(http.StatusBadRequest)))
				return
			}

			if strings.TrimSpace(splitHeader[1]) != auth {
				w.WriteHeader(http.StatusForbidden)
				w.Write([]byte(http.StatusText(http.StatusForbidden)))
				return
			}

			next.ServeHTTP(w, r)
		})
	}
}

func CORSHandler() func(next http.Handler) http.Handler {
	return cors.Handler(cors.Options{
		// Allow options requests to pass through to the middlewares
		AllowedOrigins: []string{"http://localhost:5173"},
		AllowedMethods: []string{
			"GET",
			"POST",
			"OPTIONS",
		},
		// Default go-chi headers + our custom header for identifying source
		AllowedHeaders: []string{
			"Origin",
			"Accept",
			"Content-Type",
			"If-None-Match",
		},
	})
}
