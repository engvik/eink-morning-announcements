package news

// Item is one headline and the label of the feed it came from.
type Item struct {
	Source string `json:"source"`
	Title  string `json:"title"`
}

// Items is ordered as the display should show them.
type Items []Item

// interleave takes the top item of every feed, then the second of every feed,
// and so on, so each feed leads with its top headline.
func interleave(feeds []Items) Items {
	var items Items

	for i := 0; ; i++ {
		added := false

		for _, feed := range feeds {
			if i < len(feed) {
				items = append(items, feed[i])
				added = true
			}
		}

		if !added {
			return items
		}
	}
}
