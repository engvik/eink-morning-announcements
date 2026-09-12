package transport

import (
	"net/http"
	"strings"
)

func NewAuthMiddleware(auth string) func(next http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
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

const (
	corsAllowedOrigin  = "http://localhost:5173"
	corsAllowedMethods = "GET, POST, OPTIONS"
	corsAllowedHeaders = "Origin, Accept, Content-Type, If-None-Match"
)

func CORSHandler() func(next http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			preflight := r.Method == http.MethodOptions && r.Header.Get("Access-Control-Request-Method") != ""

			w.Header().Add("Vary", "Origin")
			if r.Header.Get("Origin") == corsAllowedOrigin {
				w.Header().Set("Access-Control-Allow-Origin", corsAllowedOrigin)
				if preflight {
					w.Header().Set("Access-Control-Allow-Methods", corsAllowedMethods)
					w.Header().Set("Access-Control-Allow-Headers", corsAllowedHeaders)
				}
			}

			// Answer preflights here so they never reach auth
			if preflight {
				w.WriteHeader(http.StatusNoContent)
				return
			}

			next.ServeHTTP(w, r)
		})
	}
}
