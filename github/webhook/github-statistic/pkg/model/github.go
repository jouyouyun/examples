package model

import (
	"encoding/json"
)

type userInfo struct {
	ID    int64  `json:"id"`
	Login string `json:"login"`
}

type repositoryInfo struct {
	ID    int64    `json:"id"`
	Name  string   `json:"name"`
	Owner userInfo `json:"owner"`
}

func jsonMarshal(info interface{}) []byte {
	data, _ := json.Marshal(info)
	return data
}
