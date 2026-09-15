package news

import (
	"bytes"
	"encoding/xml"
	"fmt"
	"html"
	"strings"

	"golang.org/x/net/html/charset"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

// document covers RSS 2.0, RSS 1.0 and Atom. Tags without a namespace match
// any, so one struct reads all three.
type document struct {
	Channel struct {
		Items []entry `xml:"item"`
	} `xml:"channel"`
	Items   []entry `xml:"item"`
	Entries []entry `xml:"entry"`
}

type entry struct {
	Title string `xml:"title"`
}

type Parser struct {
	Items int
}

func NewParser(cfg *config.Config) *Parser {
	return &Parser{
		Items: cfg.NewsFetchItems,
	}
}

// Parse returns the feed's top items in the order it lists them.
func (p *Parser) Parse(feed []byte, label string) (Items, error) {
	// Feeds in the wild carry HTML entities, bare ampersands and legacy charsets.
	decoder := xml.NewDecoder(bytes.NewReader(feed))
	decoder.Strict = false
	decoder.Entity = xml.HTMLEntity
	decoder.CharsetReader = charset.NewReaderLabel

	var doc document

	if err := decoder.Decode(&doc); err != nil {
		return nil, fmt.Errorf("Error unmarshaling XML: %w", err)
	}

	entries := doc.Channel.Items
	entries = append(entries, doc.Items...)
	entries = append(entries, doc.Entries...)

	items := make(Items, 0, p.Items)

	for _, e := range entries {
		if len(items) == p.Items {
			break
		}

		// Some feeds escape their titles twice.
		title := strings.Join(strings.Fields(html.UnescapeString(e.Title)), " ")
		if title == "" {
			continue
		}

		items = append(items, Item{Source: label, Title: title})
	}

	return items, nil
}
