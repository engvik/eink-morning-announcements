package calendar

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type service interface {
	GetCalendarEvents(context.Context) (Events, error)
}

func NewHTTPHandler(cfg *config.Config, s service) http.Handler {
	r := chi.NewRouter()

	h := &handler{service: s, numEvents: cfg.CalendarFetchEvents}

	r.Get("/", h.getCalendarEvents)

	return r
}

type handler struct {
	service   service
	numEvents int
}

// response carries the next few events, plus how many are upcoming in total so
// the display can say more than it shows.
type response struct {
	Events Events `json:"events"`
	Total  int    `json:"total"`
}

func (h *handler) getCalendarEvents(w http.ResponseWriter, r *http.Request) {
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
