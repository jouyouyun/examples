package templates

import (
	"fmt"
	"io/ioutil"
	"path/filepath"
	"plugin"
)

type ModuleInfo struct {
	Name        string
	Description string
	Params      []byte
}
type ModuleInfos []*ModuleInfo

const (
	modPluginDir = "plugins"
	configPath   = "xxx"
)

var (
	errModNotFound   = fmt.Errorf("not found this module")
	errInvalidObject = fmt.Errorf("invalid obj: not Templates type")

	modPlgSet = make(map[string]*ModuleInfo)

	runModSet = make(map[string]Templates)
)

func ListModule(dir string) (ModuleInfos, error) {
	finfos, err := ioutil.ReadDir(dir)
	if err != nil {
		return nil, err
	}

	var infos ModuleInfos
	for _, finfo := range finfos {
		if finfo.IsDir() {
			continue
		}

		info, err := loadPlugin(filepath.Join(dir, finfo.Name()))
		if err != nil {
			fmt.Printf("Failed to load '%s': %v\n", finfo.Name(), err)
			continue
		}
		infos = append(infos, info)
		modPlgSet[info.Name] = info
	}
	return infos, nil
}

func NewModule(name string, params []byte) error {
	info, ok := modPlgSet[name]
	if !ok {
		return errModNotFound
	}

	// save params to config
	return info.Save(configPath)
}

func StartModule(uuid string) error {
	// load config

	var params []byte
	// get params by uuid

	var temp Templates
	// get obj by load plugin

	instance, err := temp.Start(params)
	if err != nil {
		return err
	}
	runModSet[uuid] = instance
	return nil
}

func StopModule(uuid string) error {
	temp, ok := runModSet[uuid]
	if !ok {
		return errModNotFound
	}

	err := temp.Stop()
	if err != nil {
		return err
	}
	delete(runModSet, uuid)
	return nil
}

func (info *ModuleInfo) Save(filename string) error {
	// generate uuid
	return nil
}

func loadPlugin(filename string) (*ModuleInfo, error) {
	plg, err := plugin.Open(filename)
	if err != nil {
		return nil, err
	}

	obj, err := plg.Lookup("Obj")
	if err != nil {
		return nil, err
	}

	temp, ok := obj.(*Templates)
	if !ok {
		return nil, errInvalidObject
	}

	params, err := (*temp).Parameters()
	if err != nil {
		return nil, err
	}

	return &ModuleInfo{
		Name:        (*temp).Name(),
		Description: (*temp).Description(),
		Params:      params,
	}, nil
}
