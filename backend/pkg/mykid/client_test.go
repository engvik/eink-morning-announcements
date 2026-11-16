package mykid

import (
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"testing"
	"time"
)

// fakeMyKid mimics the login and child-switch flow, holding one session cookie.
type fakeMyKid struct {
	t        *testing.T
	password string
	logins   int
	kids     []string
}

func (f *fakeMyKid) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	loggedIn := false
	if c, err := r.Cookie("PHPSESSID"); err == nil && c.Value == "valid" {
		loggedIn = true
	}

	switch {
	case r.URL.Path == "/nb/logg_inn":
		http.SetCookie(w, &http.Cookie{Name: "PHPSESSID", Value: "anon", Path: "/"})
		serveFile(f.t, w, filepath.Join(testDir, "logg_inn.html"))
	case r.URL.Path == "/forside/forside/login" && r.Method == http.MethodPost:
		f.logins++

		if r.PostFormValue("_csrf_token") != "logintoken" || r.PostFormValue("pp") != "47" ||
			r.PostFormValue("m") != "12345678" || r.PostFormValue("p") != f.password ||
			r.Header.Get("X-Requested-With") != "XMLHttpRequest" {
			w.Write([]byte(`{"status":"bad","message":"Feil brukernavn og\/eller passord."}`))
			return
		}

		http.SetCookie(w, &http.Cookie{Name: "PHPSESSID", Value: "valid", Path: "/"})
		w.Write([]byte(`{"status":"ok"}`))
	case r.URL.Path == "/_ajax/avdelinger/bytt_barn/100001/foreldre":
		f.kids = append(f.kids, "100001")
		http.Redirect(w, r, "/foreldre", http.StatusFound)
	case r.URL.Path == "/foreldre":
		if !loggedIn {
			serveFile(f.t, w, filepath.Join(testDir, "logg_inn.html"))
			return
		}

		serveFile(f.t, w, filepath.Join(testDir, "foreldre.html"))
	default:
		http.NotFound(w, r)
	}
}

func serveFile(t *testing.T, w http.ResponseWriter, path string) {
	b, err := os.ReadFile(path)
	if err != nil {
		t.Error(err)
	}

	w.Write(b)
}

func newTestClient(t *testing.T, password string) (*Client, *fakeMyKid) {
	fake := &fakeMyKid{t: t, password: "secret"}
	srv := httptest.NewServer(fake)
	t.Cleanup(srv.Close)

	c := NewClient(srv.Client(), "12345678", password, time.UTC)
	c.BaseURL = srv.URL

	return c, fake
}

func TestClientNotices(t *testing.T) {
	c, fake := newTestClient(t, "secret")

	notices, err := c.Notices(t.Context(), "100001")
	if err != nil {
		t.Fatal(err)
	}

	if len(notices) != 2 {
		t.Errorf("got %d notices, want 2", len(notices))
	}

	// The session is reused.
	if _, err := c.Notices(t.Context(), "100001"); err != nil {
		t.Fatal(err)
	}

	if fake.logins != 1 {
		t.Errorf("logins = %d, want 1", fake.logins)
	}
}

func TestClientWrongPassword(t *testing.T) {
	c, fake := newTestClient(t, "wrong")

	if _, err := c.Notices(t.Context(), "100001"); err == nil {
		t.Fatal("err = nil, want login error")
	}

	if fake.logins != 1 {
		t.Errorf("logins = %d, want 1", fake.logins)
	}
}

func TestClientUnknownKid(t *testing.T) {
	c, _ := newTestClient(t, "secret")

	if _, err := c.Notices(t.Context(), "1"); err == nil {
		t.Fatal("err = nil, want HTTP status error")
	}
}
