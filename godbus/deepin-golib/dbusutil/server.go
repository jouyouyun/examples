package main

import (
	"fmt"
	"os"

	"github.com/godbus/dbus"
	"pkg.deepin.io/lib/dbusutil"
)

const (
	dbusSender = "com.deepin.daemon.Policy"
	dbusPath   = "/com/deepin/daemon/Policy"
	dbusIFC    = dbusSender
)

type ActionType int32
type AppraiseType int32
type PolicyType int32
type Policy struct {
	Action   int32  `json:"action"`
	Appraise int32  `json:"appraise"`
	Type     int32  `json:"type"`
	Target   string `json:"target"`
}
type PolicyList []*Policy

type Service struct {
	conn    *dbusutil.Service
	methods *struct {
		GetPolicy     func() `out:"infos"`
		AddPolicy     func() `in:"info"`
		AddPolicyList func() `in:"infos"`
	}
}

func main() {
	srv, err := NewService()
	if err != nil {
		fmt.Println("Failed to new service:", err)
		os.Exit(-1)
	}

	srv.loop()
}

func NewService() (*Service, error) {
	var srv Service
	var err error
	srv.conn, err = dbusutil.NewSessionService()
	if err != nil {
		return nil, err
	}

	owned, err := srv.conn.NameHasOwner(dbusSender)
	if err != nil {
		return nil, err
	}
	if owned {
		return nil, fmt.Errorf("The service '%s' has been owned", dbusSender)
	}

	err = srv.conn.Export(dbusPath, &srv)
	if err != nil {
		return nil, err
	}

	err = srv.conn.RequestName(dbusSender)
	if err != nil {
		return nil, err
	}

	return &srv, nil
}

func (srv *Service) GetInterfaceName() string {
	return dbusIFC
}

var _infos = PolicyList{
	&Policy{
		Action:   1,
		Appraise: 1,
		Type:     1,
		Target:   "test1",
	},
	&Policy{
		Action:   1,
		Appraise: 1,
		Type:     1,
		Target:   "test2",
	},
}

func (srv *Service) GetPolicy() (PolicyList, *dbus.Error) {
	return _infos, nil
}

func (srv *Service) AddPolicy(info *Policy) *dbus.Error {
	_infos = append(_infos, info)
	return nil
}

// AddPolicyList infos type MUST BE '[]Policy', shouldn't 'PolicyList',
// othersize will have error occurred: 'Invalid type / number of args'
func (srv *Service) AddPolicyList(infos []Policy) *dbus.Error {
	for _, info := range infos {
		_infos = append(_infos, &info)
	}
	return nil
}

func (srv *Service) loop() {
	srv.conn.Wait()
}
