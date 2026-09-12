package meta

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/server"
	"github.com/engvik/eink-morning-announcements/backend/pkg/weather"
)

type service interface {
	GetSun(context.Context) (weather.Sun, error)
}

func NewHTTPHandler(cfg *config.Config, s service) *Handler {
	return &Handler{location: cfg.Location, service: s}
}

type Handler struct {
	location string
	service  service
}

func (h *Handler) Routes() []server.Route {
	return []server.Route{
		{Method: http.MethodGet, Path: "/meta", Handler: h.getMeta},
	}
}

func (h *Handler) getMeta(w http.ResponseWriter, r *http.Request) {
	sun, err := h.service.GetSun(r.Context())
	if err != nil {
		log.Printf("error getting sun: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	meta, err := GetMeta(h.location, sun)
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
