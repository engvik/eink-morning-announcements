package mykid

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/http/cookiejar"
	"net/url"
	"strings"
	"sync"
	"time"
)

const DefaultBaseURL = "https://mykid.no"

var errLoggedOut = errors.New("session logged out")

// Client is a logged-in MyKid session.
type Client struct {
	BaseURL   string
	UserAgent string

	http     *http.Client
	phone    string
	password string
	location *time.Location

	// Active child is session state.
	mu sync.Mutex
}

// NewClient copies base and gives the copy its own cookie jar.
func NewClient(base *http.Client, phone, password string, loc *time.Location) *Client {
	jar, _ := cookiejar.New(nil) // never fails without options
	c := *base
	c.Jar = jar

	return &Client{
		BaseURL:  DefaultBaseURL,
		http:     &c,
		phone:    phone,
		password: password,
		location: loc,
	}
}

// Notices returns the post-its shown for kid, logging in again once if the session expired.
func (c *Client) Notices(ctx context.Context, kid string) ([]Notice, error) {
	c.mu.Lock()
	defer c.mu.Unlock()

	notices, err := c.notices(ctx, kid)
	if !errors.Is(err, errLoggedOut) {
		return notices, err
	}

	if err := c.login(ctx); err != nil {
		return nil, fmt.Errorf("login: %w", err)
	}

	return c.notices(ctx, kid)
}

func (c *Client) notices(ctx context.Context, kid string) ([]Notice, error) {
	// Switching child redirects to that child's parent page.
	body, err := c.do(ctx, http.MethodGet, "/_ajax/avdelinger/bytt_barn/"+url.PathEscape(kid)+"/foreldre", nil)
	if err != nil {
		return nil, err
	}

	notices, loggedIn, err := parsePage(bytes.NewReader(body), c.location)
	if err != nil {
		return nil, err
	}

	if !loggedIn {
		return nil, errLoggedOut
	}

	return notices, nil
}

func (c *Client) login(ctx context.Context) error {
	page, err := c.do(ctx, http.MethodGet, "/nb/logg_inn", nil)
	if err != nil {
		return err
	}

	token, err := loginToken(bytes.NewReader(page))
	if err != nil {
		return err
	}

	form := url.Values{
		"_csrf_token": {token},
		"pp":          {"47"},
		"m":           {c.phone},
		"p":           {c.password},
	}

	body, err := c.do(ctx, http.MethodPost, "/forside/forside/login", form)
	if err != nil {
		return err
	}

	var res struct {
		Status  string `json:"status"`
		Message string `json:"message"`
	}

	if err := json.Unmarshal(body, &res); err != nil {
		return fmt.Errorf("decoding login response: %w", err)
	}

	// Only "bad" is known; a wrong success surfaces as a logged-out next page.

	if res.Status == "bad" {
		return fmt.Errorf("rejected: %s", res.Message)
	}

	return nil
}

func (c *Client) do(ctx context.Context, method, path string, form url.Values) ([]byte, error) {
	var body io.Reader
	if form != nil {
		body = strings.NewReader(form.Encode())
	}

	req, err := http.NewRequestWithContext(ctx, method, c.BaseURL+path, body)
	if err != nil {
		return nil, err
	}

	if c.UserAgent != "" {
		req.Header.Set("User-Agent", c.UserAgent)
	}

	if form != nil {
		req.Header.Set("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8")
		req.Header.Set("X-Requested-With", "XMLHttpRequest")
	}

	resp, err := c.http.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	b, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("%s %s: unexpected HTTP status %d", method, path, resp.StatusCode)
	}

	return b, nil
}
