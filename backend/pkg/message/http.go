package message

import (
	"context"
	"encoding/json"
	"errors"
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"
)

var ErrNoMessages = errors.New("No messages")

type service interface {
	SetMessage(context.Context, Message) error
	GetMessage(context.Context) (Message, error)
}

func NewHTTPHandler(s service) http.Handler {
	r := chi.NewRouter()

	h := &handler{service: s}

	r.Get("/", h.getMessage)
	r.Post("/", h.setMessage)

	return r
}

type handler struct {
	service service
}

func (h *handler) getMessage(w http.ResponseWriter, r *http.Request) {
	message, err := h.service.GetMessage(r.Context())
	if errors.Is(err, ErrNoMessages) {
		w.WriteHeader(http.StatusNotFound)
		w.Write([]byte(http.StatusText(http.StatusNotFound)))
		return
	}

	if err != nil {
		log.Printf("error getting message: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	res, err := json.Marshal(message)
	if err != nil {
		log.Printf("error marshaling message: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.Write(res)
}

func (h *handler) setMessage(w http.ResponseWriter, r *http.Request) {
	var m Message

	if err := json.NewDecoder(r.Body).Decode(&m); err != nil {
		log.Printf("error unmarshaling message: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	if err := h.service.SetMessage(r.Context(), m); err != nil {
		if errors.Is(err, ErrEmptyMessage) {
			w.WriteHeader(http.StatusBadRequest)
			w.Write([]byte(http.StatusText(http.StatusBadRequest)))
			return
		}

		log.Printf("error saving message: %s\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(http.StatusText(http.StatusInternalServerError)))
		return
	}

	w.Write([]byte(http.StatusText(http.StatusOK)))
}
