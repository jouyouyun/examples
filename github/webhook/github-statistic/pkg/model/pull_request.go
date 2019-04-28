package model

import (
	"time"
)

// PullRequest github webhook pull_request event
type PullRequest struct {
	Action      string          `json:"action"` // such as: opened, closed, reopened...
	Number      int             `json:"number"`
	PullRequest pullRequestInfo `json:"pull_request"`
	Repository  repositoryInfo  `json:"repository"`
}

// PullRequestReview github webhook pull_request_review event
type PullRequestReview struct {
	Action      string          `json:"action"` // such as: submmited, edited, dismissed
	Review      reviewInfo      `json:"review"`
	PullRequest pullRequestInfo `json:"pull_request"`
	Repository  repositoryInfo  `json:"repository"`
}

// PullRequestReviewComment github webhook pull_request_review_comment event
type PullRequestReviewComment struct {
	Action  string `json:"action"` // such as: created, deleted, edited
	Comment struct {
		ID                  int64     `json:"id"`
		PullRequestReviewID int64     `json:"pull_request_review_id"`
		URL                 string    `json:"url"`
		PullRequestURL      string    `json:"pull_request_url"`
		CreatedAt           time.Time `json:"created_at"`
		UpdatedAt           time.Time `json:"updated_at"`
		User                userInfo  `json:"user"`
	} `json:"comment"`
	PullRequest pullRequestInfo `json:"pull_request"`
	Repository  repositoryInfo  `json:"repository"`
}

type pullRequestInfo struct {
	ID        int64     `json:"id"`
	Number    int       `json:"number"`
	URL       string    `json:"url"`
	PatchURL  string    `json:"patch_url"`
	IssueURL  string    `json:"issue_url"`
	State     string    `json:"state"` // such as: open, close
	Body      string    `json:"body"`
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
	ClosedAt  time.Time `json:"closed_at"`
	MergedAt  time.Time `json:"merged_at"`
	User      userInfo  `json:"user"`
}

type reviewInfo struct {
	ID             int64     `json:"id"`
	State          string    `json:"state"`
	CommitID       string    `json:"commit_id"`
	SubmittedAt    time.Time `json:"submitted_at"`
	PullRequestURL string    `json:"pull_request_url"`
	User           userInfo  `json:"user"`
}

func (info *PullRequest) Dump() []byte {
	return jsonMarshal(info)
}

func (info *PullRequestReview) Dump() []byte {
	return jsonMarshal(info)
}

func (info *PullRequestReviewComment) Dump() []byte {
	return jsonMarshal(info)
}
