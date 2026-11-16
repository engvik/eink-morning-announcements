package mykid

import (
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"
	"time"
)

// testDir holds fixtures under backend/test.
const testDir = "../../test/mykid"

func TestParsePage(t *testing.T) {
	oslo, err := time.LoadLocation("Europe/Oslo")
	if err != nil {
		t.Fatal(err)
	}

	f, err := os.Open(filepath.Join(testDir, "foreldre.html"))
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	notices, loggedIn, err := parsePage(f, oslo)
	if err != nil {
		t.Fatal(err)
	}

	if !loggedIn {
		t.Error("loggedIn = false, want true")
	}

	want := []Notice{
		{ID: "900001", Date: time.Date(2026, 11, 9, 0, 0, 0, 0, oslo), Heading: "Barnehagen"},
		{ID: "900002", Date: time.Date(2026, 11, 12, 0, 0, 0, 0, oslo), Heading: "Avdelingen"},
	}

	if !slices.EqualFunc(notices, want, func(a, b Notice) bool {
		return a.ID == b.ID && a.Date.Equal(b.Date) && a.Heading == b.Heading
	}) {
		t.Errorf("notices = %+v, want %+v", notices, want)
	}
}

func TestParsePageErrors(t *testing.T) {
	tests := []struct {
		name         string
		page         string
		wantErr      bool
		wantLoggedIn bool
	}{
		{name: "login page is logged out", page: `<form id="login_form"></form>`},
		{name: "no post-its", page: `<meta name="_csrf_token" content="x">`, wantLoggedIn: true},
		{name: "missing id", page: `<div class="newnote"><div class="dato">09.11.2026</div></div>`, wantErr: true},
		{name: "missing date", page: `<div class="newnote" data-id="1"></div>`, wantErr: true},
		{name: "bad date", page: `<div class="newnote" data-id="1"><div class="dato">2026-11-09</div></div>`, wantErr: true},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			_, loggedIn, err := parsePage(strings.NewReader(tt.page), time.UTC)
			if (err != nil) != tt.wantErr {
				t.Fatalf("err = %v, wantErr %v", err, tt.wantErr)
			}

			if !tt.wantErr && loggedIn != tt.wantLoggedIn {
				t.Errorf("loggedIn = %v, want %v", loggedIn, tt.wantLoggedIn)
			}
		})
	}
}

func TestLoginToken(t *testing.T) {
	f, err := os.Open(filepath.Join(testDir, "logg_inn.html"))
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	got, err := loginToken(f)
	if err != nil {
		t.Fatal(err)
	}

	if got != "logintoken" {
		t.Errorf("loginToken = %q, want %q", got, "logintoken")
	}
}
