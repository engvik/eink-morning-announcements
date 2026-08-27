package meta

import (
	"encoding/json"
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

func NewHTTPHandler(cfg *config.Config) http.Handler {
	r := chi.NewRouter()

	h := &handler{location: cfg.Location}

	r.Get("/", h.getMeta)

	return r
}

type handler struct {
	location string
}

func (h *handler) getMeta(w http.ResponseWriter, r *http.Request) {
	meta, err := GetMeta(h.location)
	if err != nil {
		log.Printf("error setting location: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(meta)
	if err != nil {
		log.Printf("error marshaling message: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}
