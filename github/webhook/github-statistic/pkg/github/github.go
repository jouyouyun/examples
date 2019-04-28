package github

import (
	"encoding/json"
	"github-statistic/internal/log"
	"github-statistic/pkg/model"
)

type Event interface {
	Dump() []byte
}

const (
	EventIssueComment             string = "issue_comment"
	EventIssues                          = "issues"
	EventPullRequest                     = "pull_request"
	EventPullRequestReview               = "pull_request_review"
	EventPullRequestReviewComment        = "pull_request_review_comment"
	EventStar                            = "star"
)

func HandleEvent(ty string, data []byte) error {
	var (
		event Event
		err   error
	)

	switch ty {
	case EventIssueComment:
		var info *model.IssueComment
		info, err = bindIssueComment(data)
		event = info
	case EventIssues:
		var info *model.Issues
		info, err = bindIssues(data)
		event = info
	case EventPullRequest:
		var info *model.PullRequest
		info, err = bindPullRequest(data)
		event = info
	case EventPullRequestReview:
		var info *model.PullRequestReview
		info, err = bindPullRequestReview(data)
		event = info
	case EventPullRequestReviewComment:
		var info *model.PullRequestReviewComment
		info, err = bindPullRequestReviewComment(data)
		event = info
	case EventStar:
		var info *model.Star
		info, err = bindStar(data)
		event = info
	}
	if err != nil {
		return err
	}

	log.Info("[Event] will handle:", string(event.Dump()))
	return nil
}

func bindIssueComment(data []byte) (*model.IssueComment, error) {
	var info model.IssueComment
	err := json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func bindIssues(data []byte) (*model.Issues, error) {
	var info model.Issues
	err := json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func bindPullRequest(data []byte) (*model.PullRequest, error) {
	var info model.PullRequest
	err := json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func bindPullRequestReview(data []byte) (*model.PullRequestReview, error) {
	var info model.PullRequestReview
	err := json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func bindPullRequestReviewComment(data []byte) (*model.PullRequestReviewComment, error) {
	var info model.PullRequestReviewComment
	err := json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func bindStar(data []byte) (*model.Star, error) {
	var info model.Star
	err := json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}
