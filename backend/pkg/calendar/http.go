package calendar

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/server"
)

type service interface {
	GetCalendarEvents(context.Context) (Events, error)
}

func NewHTTPHandler(cfg *config.Config, s service) *Handler {
	return &Handler{service: s, numEvents: cfg.CalendarFetchEvents}
}

type Handler struct {
	service   service
	numEvents int
}

func (h *Handler) Routes() []server.Route {
	return []server.Route{
		{Method: http.MethodGet, Path: "/calendar", Handler: h.getCalendarEvents},
	}
}

// response carries the next few events, plus how many are upcoming in total so
// the display can say more than it shows.
type response struct {
	Events Events `json:"events"`
	Total  int    `json:"total"`
}

func (h *Handler) getCalendarEvents(w http.ResponseWriter, r *http.Request) {
	events, err := h.service.GetCalendarEvents(r.Context())
	if err != nil {
		log.Printf("error getting events: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(response{
		Events: events.Limit(h.numEvents),
		Total:  len(events),
	})
	if err != nil {
		log.Printf("error marshaling events: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}
