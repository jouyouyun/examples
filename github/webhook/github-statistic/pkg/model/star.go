package model

import (
	"time"
)

// Star github webhook start event
type Star struct {
	Action     string         `json:"action"` // such as: created, deleted
	StarredAt  time.Time      `json:"starred_at"`
	Repository repositoryInfo `json:"repository"`
}

func (info *Star) Dump() []byte {
	return jsonMarshal(info)
}
