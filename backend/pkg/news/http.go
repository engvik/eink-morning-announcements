package news

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/server"
)

type service interface {
	GetNews(context.Context) (Items, error)
}

func NewHTTPHandlers(s service) *Handler {
	return &Handler{service: s}
}

type Handler struct {
	service service
}

func (h *Handler) Routes() []server.Route {
	return []server.Route{
		{Method: http.MethodGet, Path: "/news", Handler: h.getNews},
	}
}

type response struct {
	Items Items `json:"items"`
}

func (h *Handler) getNews(w http.ResponseWriter, r *http.Request) {
	items, err := h.service.GetNews(r.Context())
	if err != nil {
		log.Printf("error getting news: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(response{Items: items})
	if err != nil {
		log.Printf("error marshaling news: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}
