package storage

type store interface {
	SetCalendarEvents()
	GetCalendarEvents()
}

type Storage struct {
	Client store
}

func New(c store) *Storage {
	return &Storage{
		Client: c,
	}
}
