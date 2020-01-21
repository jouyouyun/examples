package main

import (
	"fmt"
	"os/exec"
	"strconv"
	"strings"
)

type ModeFlag int32

const (
	ddeWLOutputCmd = "dde_wloutput"
)

const (
	ModeFlagNone      ModeFlag = 0
	ModeFlagCurrent   ModeFlag = 1 << 0
	ModeFlagPreferred ModeFlag = 1 << 1
)

func (f ModeFlag) String() string {
	switch f {
	case ModeFlagNone:
		return "none"
	case ModeFlagCurrent:
		return "current"
	case ModeFlagPreferred:
		return "preferred"
	}
	return "unknown"
}

type Output struct {
	Name         string
	UUID         string
	Manufacturer string

	Enabled bool
	Primary bool

	X          int32
	Y          int32
	Width      int32
	Height     int32
	PhysWidth  int32
	PhysHeight int32
	Transform  int32

	Refresh float64
	ScaleF  float64

	Modes OutputModeList
}
type OutputList []*Output

type OutputMode struct {
	Width  int32
	Height int32
	Flag   ModeFlag

	Refresh float64
}
type OutputModeList []*OutputMode

func (list OutputList) ScreenSize() (int32, int32) {
	var width int32
	var height int32
	for _, out := range list {
		w := out.X + out.Width
		h := out.Y + out.Height
		if width < w {
			width = w
		}
		if height < h {
			height = h
		}
	}
	return width, height
}

func (modes OutputModeList) Current() *OutputMode {
	for _, m := range modes {
		if (m.Flag & ModeFlagCurrent) == ModeFlagCurrent {
			return m
		}
	}
	return nil
}

func (modes OutputModeList) Preferred() *OutputMode {
	for _, m := range modes {
		if (m.Flag & ModeFlagPreferred) == ModeFlagPreferred {
			return m
		}
	}
	return nil
}

type ScreenInfo struct {
	Width  int32
	Height int32

	Outputs OutputList
}

func main() {
	info, err := GetScreenInfo()
	if err != nil {
		fmt.Println("Failed to get screen info:", err)
		return
	}

	fmt.Println("Screen info:", info.Width, info.Height)
	for _, o := range info.Outputs {
		fmt.Printf("\t%#v\n", o)
	}
}

func GetScreenInfo() (*ScreenInfo, error) {
	data, err := exec.Command(ddeWLOutputCmd, "get").CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("%s(%s)", string(data), err)
	}

	list, err := parseWLOutputData(data)
	if err != nil {
		return nil, err
	}
	w, h := list.ScreenSize()

	return &ScreenInfo{
		Width:   w,
		Height:  h,
		Outputs: list,
	}, nil
}

func SetOutputs(list OutputList) error {
	for _, info := range list {
		err := doSetOutput(info)
		if err != nil {
			return err
		}
	}
	return nil
}

func doSetOutput(info *Output) error {
	var enabled = 1
	if !info.Enabled {
		enabled = 0
	}

	data, err := exec.Command(ddeWLOutputCmd, "set", info.Name, fmt.Sprintf("%d", enabled),
		fmt.Sprintf("%d", info.X), fmt.Sprintf("%d", info.Y), fmt.Sprintf("%d", info.Width),
		fmt.Sprintf("%d", info.Height), fmt.Sprintf("%d", int32(info.Refresh*1000)),
		fmt.Sprintf("%d", info.Transform)).CombinedOutput()
	if err != nil {
		return fmt.Errorf("%s(%s)", string(data), err)
	}

	return nil
}

func parseWLOutputData(data []byte) (OutputList, error) {
	var list OutputList
	var info *Output
	var lines = strings.Split(string(data), "\n")
	for _, line := range lines {
		if len(line) == 0 {
			if info != nil {
				// TODO(jouyouyun): improve primary check rule
				if info.Enabled && (info.X == 0 && info.Y == 0) {
					info.Primary = true
				}
				list = append(list, info)
			}
			continue
		}
		if line[0] != '\t' {
			info = &Output{}
			err := parseWLOutputFirstLine(info, line)
			if err != nil {
				return nil, err
			}
		} else {
			mode, err := parsetWLOutputModeLine(line)
			if err != nil {
				return nil, err
			}
			info.Modes = append(info.Modes, mode)
		}
	}
	return list, nil
}

func parseWLOutputFirstLine(info *Output, line string) error {
	items := strings.Split(line, " ")
	if len(items) != 9 {
		return fmt.Errorf("invalid output first line: %s, items: %d", line, len(items))
	}

	info.Name = items[0]
	info.UUID = items[7]
	info.Manufacturer = items[8]

	if items[1] == "enabled" {
		info.Enabled = true
	}

	if err := parseWLOutputPosition(info, items[2]); err != nil {
		return err
	}

	if err := strToFloat64(&info.Refresh, items[3], "invlaid output first line: %s"); err != nil {
		return err
	}

	if err := strToInt32(&info.Transform, items[4], "invlaid output first line: %s"); err != nil {
		return err
	}

	if err := strToFloat64(&info.ScaleF, items[5], "invlaid output first line: %s"); err != nil {
		return err
	}

	physList := strings.Split(items[6], "x")
	if len(physList) != 2 {
		return fmt.Errorf("invlaid output first line: %s, phys items: %d", line, len(physList))
	}
	if err := strToInt32(&info.PhysWidth, physList[0], "invlaid output first line: %s"); err != nil {
		return err
	}
	if err := strToInt32(&info.PhysHeight, physList[1], "invlaid output first line: %s"); err != nil {
		return err
	}

	return nil
}

func parsetWLOutputModeLine(line string) (*OutputMode, error) {
	line = strings.TrimLeft(line, "\t")
	items := strings.Split(line, "\t")
	if len(items) < 2 {
		return nil, fmt.Errorf("invalid mode line: %s, items: %d", line, len(items))
	}

	sizeList := strings.Split(items[0], "x")
	if len(sizeList) != 2 {
		return nil, fmt.Errorf("invalid mode line: %s, size items: %d", line, len(sizeList))
	}

	var info OutputMode
	if err := strToInt32(&info.Width, sizeList[0], "invalid mode line: %s"); err != nil {
		return nil, err
	}
	if err := strToInt32(&info.Height, sizeList[1], "invalid mode line: %s"); err != nil {
		return nil, err
	}

	if err := strToFloat64(&info.Refresh, items[1], "invalid mode line: %s"); err != nil {
		return nil, err
	}

	if len(items) == 3 {
		if items[2] == ModeFlagCurrent.String() {
			info.Flag &= ModeFlagCurrent
		}
		if items[2] == ModeFlagPreferred.String() {
			info.Flag &= ModeFlagPreferred
		}
	}
	if len(items) == 4 {
		if items[2] == ModeFlagCurrent.String() {
			info.Flag &= ModeFlagCurrent
		}
		if items[3] == ModeFlagPreferred.String() {
			info.Flag &= ModeFlagPreferred
		}
	}
	return &info, nil
}

func parseWLOutputPosition(info *Output, str string) error {
	items := strings.Split(str, "+")
	if len(items) != 3 {
		return fmt.Errorf("invalid output first position: %s, items: %d", str, len(items))
	}
	sizeList := strings.Split(items[0], "x")
	if len(sizeList) != 2 {
		return fmt.Errorf("invalid output first position: %s, items: %d", str, len(sizeList))
	}

	if err := strToInt32(&info.X, sizeList[0], "invalid output first position: %s"); err != nil {
		return err
	}
	if err := strToInt32(&info.Y, sizeList[1], "invalid output first position: %s"); err != nil {
		return err
	}
	if err := strToInt32(&info.Width, items[1], "invalid output first position: %s"); err != nil {
		return err
	}
	if err := strToInt32(&info.Width, items[2], "invalid output first position: %s"); err != nil {
		return err
	}

	return nil
}

func strToInt32(value *int32, str, errFmt string) error {
	v, err := strconv.Atoi(str)
	if err != nil {
		return fmt.Errorf(errFmt, err)
	}
	*value = int32(v)
	return nil
}

func strToFloat64(value *float64, str, errFmt string) error {
	v, err := strconv.ParseFloat(str, 10)
	if err != nil {
		return fmt.Errorf(errFmt, err)
	}
	*value = v
	return nil
}
