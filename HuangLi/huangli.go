/**
 * Description: 使用百度的接口查询黄历信息
 *
 * Author: jouyouyun <jouyouwen717@gmail.com>
 *
**/
package main

import (
	"flag"
	"fmt"
	"time"
)

const (
	yearMaxRange = 10
)

var (
	start  = flag.Int("s", 0, "The start year, the min value is 2008")
	end    = flag.Int("e", 0, "The end year, the max year is (now year) + 1")
	test   = flag.Bool("t", false, "Test huangli api")
	dbFile = flag.String("f", "huangli.db", "The huangli data sqlite db file")
)

func main() {
	flag.Parse()
	if *test {
		doTest()
		return
	}

	if (*start == 0 && *end == 0) || *end-*start < 0 || *start < 2008 || *end > (time.Now().Year()+1) {
		fmt.Printf("Invalid start year and end year: %d - %d\n", *start, *end)
		return
	}

	err := Init(*dbFile)
	if err != nil {
		panic(err)
	}
	defer Finalize()

	// generated db data
	var list HuangLiList
	for i := *start; i <= *end; i++ {
		for j := 1; j < 13; j++ {
			if len(list) > 100 {
				err := list.Create()
				if err != nil {
					fmt.Println("Failed to create db data:", err)
					return
				}
				list = HuangLiList{}
			}
			info, err := newBaiduHuangLiByDate(i, j)
			if err != nil {
				fmt.Println("Failed to generate huangli info:", err)
				return
			}
			list = append(list, info.ToHuangLiList()...)
		}
	}

	if len(list) == 0 {
		return
	}

	err = list.Create()
	if err != nil {
		fmt.Println("Failed to create db data:", err)
		return
	}
}

func doTest() {
	n := time.Now()
	data, err := doGet(makeURL(n.Year(), int(n.Month())))
	if err != nil {
		fmt.Println("Failed to get huangli from api:", err)
		return
	}
	info, err := newBaiduHuangLi(data)
	if err != nil {
		fmt.Println("Failed to unmarshal:", err)
		return
	}
	info.Dump()
}

func newBaiduHuangLiByDate(year, month int) (*baiduHuangLi, error) {
	data, err := doGet(makeURL(year, month))
	if err != nil {
		return nil, err
	}
	return newBaiduHuangLi(data)
}
