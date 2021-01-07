// go build -buildmode=plugin -o=libplugin_user.so -ldflags '-extldflags -Wl,-soname,plugin_user' user.go
package main

import (
	"../../../templates"
	"../../modules/user"
)

var Obj templates.Templates = &user.Manager{}
