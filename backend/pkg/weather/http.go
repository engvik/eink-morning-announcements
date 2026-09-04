package weather

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type service interface {
	GetWeatherForecasts(context.Context) (Forecasts, error)
}

func NewHTTPHandler(cfg *config.Config, s service) http.Handler {
	r := chi.NewRouter()

	h := &handler{service: s, numForecasts: cfg.WeatherFetchEorecasts}

	r.Get("/", h.getWeatherForecasts)

	return r
}

type handler struct {
	service      service
	numForecasts int
}

// response carries the next few hours for the hourly strip, plus per-day
// aggregates covering the whole series.
type response struct {
	Forecasts Forecasts `json:"forecasts"`
	Days      []Day     `json:"days"`
}

func (h *handler) getWeatherForecasts(w http.ResponseWriter, r *http.Request) {
	forecasts, err := h.service.GetWeatherForecasts(r.Context())
	if err != nil {
		log.Printf("error getting forecasts: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(response{
		Forecasts: forecasts.Limit(h.numForecasts),
		Days:      forecasts.Summarize(),
	})
	if err != nil {
		log.Printf("error marshaling forecasts: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}
