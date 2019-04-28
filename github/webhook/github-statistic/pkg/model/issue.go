package model

import (
	"time"
)

// Issues github webhook issues event
type Issues struct {
	Action     string         `json:"action"` // such as: opened, deleted, closed, reopened...
	Issue      issueInfo      `json:"issue"`
	Repository repositoryInfo `json:"repository"`
}

type IssueComment struct {
	Action  string    `json:"action"` // such as: created, edited, deleted
	Issue   issueInfo `json:"issue"`
	Comment struct {
		ID        int64     `json:"id"`
		URL       string    `json:"url"`
		IssueURL  string    `json:"issue_url"`
		User      userInfo  `json:"user"`
		CreatedAt time.Time `json:"created_at"`
		UpdatedAt time.Time `json:"updated_at"`
	} `json:"comment"`
	Repository repositoryInfo `json:"repository"`
}

type issueInfo struct {
	ID            int64    `json:"id"`
	Number        int      `json:"number"`
	Title         string   `json:"title"`
	URL           string   `json:"url"`
	RepositoryURL string   `json:"repository_url"`
	State         string   `json:"state"`
	User          userInfo `json:"user"`
}

func (info *IssueComment) Dump() []byte {
	return jsonMarshal(info)
}

func (info *Issues) Dump() []byte {
	return jsonMarshal(info)
}
