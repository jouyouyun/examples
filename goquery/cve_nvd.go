package main

import (
	"fmt"
	"os"
	"strings"

	"github.com/PuerkitoBio/goquery"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: %s <nvd url>\n", os.Args[0])
		return
	}

	dom, err := goquery.NewDocument(os.Args[1])
	if err != nil {
		fmt.Println("Failed to new document:", err)
		return
	}

	// 元素属性选择器 'element[property=value]'
	sel := dom.Find("span[data-testid=vuln-cvssv3-base-score]")
	if sel != nil {
		fmt.Println("Base score:", sel.Text())
	}

	sel = dom.Find("span[data-testid=vuln-cvssv3-base-score-severity]")
	if sel != nil {
		fmt.Println("Base score severity:", sel.Text())
	}

	sel = dom.Find("span[data-testid=vuln-cvssv3-vector]")
	if sel != nil {
		v := strings.TrimSpace(sel.Text())
		list := strings.Split(v, "\n")
		fmt.Printf("Vector: %q\n", list[0])
	}
}
