package dpkgrepo

import (
	"errors"
)

var (
	ErrHTTPNoResp = errors.New("on response recieved")
	ErrHTTPStatus = errors.New("error response status code")
)
