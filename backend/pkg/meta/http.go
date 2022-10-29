package meta

import (
	"encoding/json"
	"log"
	"net/http"

	"github.com/go-chi/chi"
)

func NewHTTPHandler() http.Handler {
	r := chi.NewRouter()

	h := &handler{}

	r.Get("/", h.getMeta)

	return r
}

type handler struct{}

func (h *handler) getMeta(w http.ResponseWriter, r *http.Request) {
	meta := GetMeta()
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
