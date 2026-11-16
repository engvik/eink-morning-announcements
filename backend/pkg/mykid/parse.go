package mykid

import (
	"fmt"
	"io"
	"strings"
	"time"

	"golang.org/x/net/html"
)

// Notice is a post-it on the parent page.
type Notice struct {
	ID      string
	Date    time.Time
	Heading string
}

// parsePage returns the post-its on a parent page, and whether the page came
// from a logged-in session.
func parsePage(r io.Reader, loc *time.Location) ([]Notice, bool, error) {
	doc, err := html.Parse(r)
	if err != nil {
		return nil, false, err
	}

	var notices []Notice
	loggedIn := false

	for n := range doc.Descendants() {
		if n.Type != html.ElementNode {
			continue
		}

		switch {
		case n.Data == "meta" && attr(n, "name") == "_csrf_token":
			loggedIn = true
		case n.Data == "div" && hasClass(n, "newnote"):
			notice, err := parseNotice(n, loc)
			if err != nil {
				return nil, loggedIn, err
			}

			notices = append(notices, notice)
		}
	}

	return notices, loggedIn, nil
}

func parseNotice(n *html.Node, loc *time.Location) (Notice, error) {
	notice := Notice{ID: attr(n, "data-id")}
	if notice.ID == "" {
		return Notice{}, fmt.Errorf("post-it without data-id")
	}

	for c := range n.Descendants() {
		switch {
		case hasClass(c, "dato"):
			date, err := time.ParseInLocation("02.01.2006", strings.TrimSpace(text(c)), loc)
			if err != nil {
				return Notice{}, fmt.Errorf("post-it %s date: %w", notice.ID, err)
			}

			notice.Date = date
		case hasClass(c, "heading"):
			notice.Heading = strings.TrimSpace(text(c))
		}
	}

	if notice.Date.IsZero() {
		return Notice{}, fmt.Errorf("post-it %s without date", notice.ID)
	}

	return notice, nil
}

// loginToken returns the CSRF token of the login form.
func loginToken(r io.Reader) (string, error) {
	doc, err := html.Parse(r)
	if err != nil {
		return "", err
	}

	for n := range doc.Descendants() {
		if n.Type == html.ElementNode && n.Data == "form" && attr(n, "id") == "login_form" {
			for c := range n.Descendants() {
				if c.Type == html.ElementNode && c.Data == "input" && attr(c, "name") == "_csrf_token" {
					return attr(c, "value"), nil
				}
			}
		}
	}

	return "", fmt.Errorf("no login form token")
}

func attr(n *html.Node, key string) string {
	for _, a := range n.Attr {
		if a.Key == key {
			return a.Val
		}
	}

	return ""
}

func hasClass(n *html.Node, class string) bool {
	return n.Type == html.ElementNode && strings.Contains(" "+attr(n, "class")+" ", " "+class+" ")
}

func text(n *html.Node) string {
	var b strings.Builder

	for c := range n.Descendants() {
		if c.Type == html.TextNode {
			b.WriteString(c.Data)
		}
	}

	return b.String()
}
