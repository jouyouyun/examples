package main

import (
	"encoding/json"
	"fmt"
)

type LshwBase struct {
	Product     string      `json:"product"`
	Description string      `json:"description"`
	LogicalName interface{} `json:"logicalname"`
	Vendor      string      `json:"vendor"`
	Version     string      `json:"version"`
	Serial      string      `json:"serial"`
	Size        int64       `json:"size"`
	Units       string      `json:"units"`
}
type LshwBaseList []*LshwBase

type Lshw struct {
	ID    string `json:"id"`
	Class string `json:"class"`
	LshwBase
	Capacity int64 `json:"capacity"`

	Children []Lshw `json:"children"`
}

const (
	DefaultLshwCmd = "lshw -json -quiet"
)

func NewLshw(data []byte) (*Lshw, error) {
	var hw Lshw
	err := json.Unmarshal(data, &hw)
	if err != nil {
		return nil, err
	}
	return &hw, nil
}

func (hw *Lshw) GetNetwork() LshwBaseList {
	return hw.getByID("network")
}

func (hw *Lshw) GetDisk() LshwBaseList {
	return hw.getByID("disk")
}

func (list LshwBaseList) Dump() {
	fmt.Println("====================")
	for _, base := range list {
		fmt.Println("Product:", base.Product)
		fmt.Println("\tdescription:", base.Description)
		fmt.Println("\tlogicalname", base.LogicalName)
		fmt.Println("\tversion:", base.Version)
		fmt.Println("\tvendor:", base.Vendor)
		fmt.Println("\tserial:", base.Serial)
		fmt.Println("\tsize:", base.Size)
		fmt.Println("\tunits:", base.Units)
	}
	fmt.Println("=======[END]========")
}

func (hw *Lshw) getByID(id string) LshwBaseList {
	var infos LshwBaseList

	if hw.ID == id {
		// Ignore usb network
		// TODO(jouyouyun): How to handle usb network?
		if len(hw.Product) == 0 {
			return nil
		}

		var info = hw.LshwBase
		infos = append(infos, &info)
		return infos
	}

	for _, child := range hw.Children {
		tmp := child.getByID(id)
		if len(tmp) == 0 {
			continue
		}
		infos = append(infos, tmp...)
	}
	return infos
}
