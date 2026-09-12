package weather

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/server"
)

type service interface {
	GetWeatherForecasts(context.Context) (Forecasts, error)
}

func NewHTTPHandler(cfg *config.Config, s service) *Handler {
	return &Handler{service: s, numForecasts: cfg.WeatherFetchEorecasts}
}

type Handler struct {
	service      service
	numForecasts int
}

func (h *Handler) Routes() []server.Route {
	return []server.Route{
		{Method: http.MethodGet, Path: "/weather", Handler: h.getWeatherForecasts},
	}
}

// response carries the next few hours for the hourly strip, plus per-day
// aggregates covering the whole series.
type response struct {
	Forecasts Forecasts `json:"forecasts"`
	Days      []Day     `json:"days"`
}

func (h *Handler) getWeatherForecasts(w http.ResponseWriter, r *http.Request) {
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
