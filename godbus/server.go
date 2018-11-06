package main

import (
	"fmt"

	"time"

	"github.com/godbus/dbus"
	"github.com/godbus/dbus/introspect"
	"github.com/godbus/dbus/prop"
)

type Server struct {
	conn *dbus.Conn

	Name string
}

const (
	dbusName = "org.jouyouyun.Test"
	dbusPath = "/org/jouyouyun/Test"
	dbusIFC  = "org.jouyouyun.Test"
)

func main() {
	s, err := newServer()
	if err != nil {
		fmt.Println("[Server] Failed to new server:", err)
		return
	}
	err = s.setupDBus()
	if err != nil {
		fmt.Println("[Server] Failed to setup bus:", err)
		return
	}
	fmt.Println("[Server] Listen on org.jouyouyun.Test...")
	time.AfterFunc(time.Second*10, func() {
		s.Name = "Test"
		s.nameChanged(s.Name)
	})
	// wait to terminate
	select {}
}

func (s *Server) Hello() (string, *dbus.Error) {
	return "Hello, world!", nil
}

func (s *Server) setupDBus() error {
	// export object
	err := s.conn.Export(s, dbusPath, dbusIFC)
	if err != nil {
		return err
	}
	props := prop.New(s.conn, dbusPath, s.makeProps())
	propsNode := &introspect.Node{
		Name: dbusName,
		Interfaces: []introspect.Interface{
			introspect.IntrospectData,
			prop.IntrospectData,
			{
				Name:       dbusIFC,
				Methods:    introspect.Methods(s),
				Properties: props.Introspection(dbusIFC),
				Signals: []introspect.Signal{
					{
						Name: "NameChanged",
						Args: []introspect.Arg{{Name: "name", Type: "s"}},
					},
				},
			},
		},
	}
	err = s.conn.Export(introspect.NewIntrospectable(propsNode), dbusPath,
		"org.freedesktop.DBus.Introspectable")
	if err != nil {
		return err
	}

	// request bus name
	// Note: must after export, otherwise will occur wrong (not found method on object) via dbus-daemon
	reply, err := s.conn.RequestName(dbusName, dbus.NameFlagDoNotQueue)
	if err != nil {
		return err
	}
	if reply != dbus.RequestNameReplyPrimaryOwner {
		return fmt.Errorf("Name '%s' has exists", dbusName)
	}
	return nil
}

func (s *Server) makeProps() map[string]map[string]*prop.Prop {
	return map[string]map[string]*prop.Prop{
		dbusIFC: {
			"Name": &prop.Prop{
				Value:    &(s.Name),
				Writable: false,
				Emit:     prop.EmitTrue,
				Callback: func(c *prop.Change) *dbus.Error {
					fmt.Println("[Server]", c.Name, "change to", c.Value)
					return nil
				},
			},
		},
	}
}

func (s *Server) nameChanged(value string) {
	err := s.conn.Emit(dbusPath, dbusIFC+".NameChanged", value)
	if err != nil {
		fmt.Println("[Server] Failed to emit:", err)
	}
}

func newServer() (*Server, error) {
	// got session bus connection, not close this connection,
	// otherwise the session bus will close, because the bus is global varaint
	conn, err := dbus.SessionBus()
	if err != nil {
		return nil, err
	}
	return &Server{
		conn: conn,
	}, nil
}
