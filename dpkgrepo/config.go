package dpkgrepo

import (
	"gopkg.in/yaml.v2"
	"io/ioutil"
)

type repoDistConfig struct {
	URI          string   `yaml:"uri"`
	Distribution string   `yaml:"distribution"`
	Components   []string `yaml:"components"`
}
type repoDistConfigList []*repoDistConfig

type RepoConfig struct {
	RepoList repoDistConfigList `yaml:"repository"`
}

func LoadRepoConfig(filename string) (*RepoConfig, error) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var rc RepoConfig
	err = yaml.Unmarshal(content, &rc)
	if err != nil {
		return nil, err
	}
	return &rc, nil
}
