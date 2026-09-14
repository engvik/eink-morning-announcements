package message

import (
	"context"
	"encoding/json"
	"errors"
	"log"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/server"
)

var ErrNoMessages = errors.New("No messages")

type service interface {
	SetMessage(context.Context, Message) error
	GetMessage(context.Context) (Message, error)
}

func NewHTTPHandlers(s service) *Handler {
	return &Handler{service: s}
}

type Handler struct {
	service service
}

func (h *Handler) Routes() []server.Route {
	return []server.Route{
		{Method: http.MethodGet, Path: "/message", Handler: h.getMessage, Public: true},
		{Method: http.MethodPost, Path: "/message", Handler: h.setMessage, Public: true},
	}
}

func (h *Handler) getMessage(w http.ResponseWriter, r *http.Request) {
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

func (h *Handler) setMessage(w http.ResponseWriter, r *http.Request) {
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
