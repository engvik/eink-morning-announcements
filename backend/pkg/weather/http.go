package weather

import (
	"context"
	"encoding/json"
	"log"
	"net/http"

	"github.com/go-chi/chi"
)

type service interface {
	GetWeatherForecasts(context.Context) ([]Forecast, error)
}

func NewHTTPHandler(s service) http.Handler {
	r := chi.NewRouter()

	h := &handler{service: s}

	r.Get("/", h.getWeatherForecasts)

	return r
}

type handler struct {
	service service
}

func (h *handler) getWeatherForecasts(w http.ResponseWriter, r *http.Request) {
	forecasts, err := h.service.GetWeatherForecasts(r.Context())
	if err != nil {
		log.Printf("error getting forecasts: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(forecasts)
	if err != nil {
		log.Printf("error marshaling forecasts: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}
