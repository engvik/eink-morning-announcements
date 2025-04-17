package calendar

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"
)

type service interface {
	GetCalendarEvents(context.Context) ([]Event, error)
}

func NewHTTPHandler(s service) http.Handler {
	r := chi.NewRouter()

	h := &handler{service: s}

	r.Get("/", h.getCalendarEvents)

	return r
}

type handler struct {
	service service
}

func (h *handler) getCalendarEvents(w http.ResponseWriter, r *http.Request) {
	events, err := h.service.GetCalendarEvents(r.Context())
	if err != nil {
		log.Printf("error getting events: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(events)
	if err != nil {
		log.Printf("error marshaling events: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}
