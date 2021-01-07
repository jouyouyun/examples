package user

import (
	"encoding/json"

	"../../../templates"
)

type User struct {
	ID   uint32
	Name string
	Tags []string
}

type Manager struct {
	DB  templates.Database
	Srv templates.Server
}

func (m *Manager) Name() string {
	return "user manager template"
}

func (m *Manager) Description() string {
	return "A template of user manager. We can create user manager module via this template"
}

func (m *Manager) Parameters() ([]byte, error) {
	var info Manager
	return json.Marshal(&info)
}

func (m *Manager) Start(params []byte) (templates.Templates, error) {
	println("Params:", string(params))
	var info Manager

	err := json.Unmarshal(params, &info)
	if err != nil {
		return nil, err
	}

	err = info.connectDB()
	if err != nil {
		return nil, err
	}
	err = info.run()
	if err != nil {
		return nil, err
	}

	return &info, nil
}

func (m *Manager) Stop() error {
	return nil
}

func (m *Manager) connectDB() error {
	return nil
}

func (m *Manager) run() error {
	return nil
}
