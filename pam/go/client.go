package main

import (
	"fmt"
	"os"

	"pkg.deepin.io/lib/pam"
)

func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <pam service> <username>\n", os.Args[0])
		return
	}

	var (
		trans *pam.Transaction
		err   error
	)

	trans, err = pam.StartFunc(os.Args[1], os.Args[2], func(style pam.Style, msg string) (string, error) {
		if trans == nil {
			fmt.Println("Null transaction")
			return "", nil
		}
		switch style {
		case pam.PromptEchoOn:
			fmt.Println("Echo on:", msg)
		case pam.PromptEchoOff:
			var buf string
			fmt.Printf("%s", msg)
			_, err := fmt.Scanf("%s", &buf)
			if err != nil {
				fmt.Println("Failed to scanf:", err)
				return "", err
			}
			return buf, nil
		case pam.TextInfo:
			fmt.Println("Text info:", msg)
		case pam.ErrorMsg:
			fmt.Println("Error:", msg)
		}
		return "", nil
	})

	if err != nil {
		fmt.Println("Failed to start:", err)
		return
	}

	err = trans.Authenticate(pam.DisallowNullAuthtok)
	if err != nil {
		fmt.Println("Failed to authenticate:", err)
	} else {
		fmt.Println("Authenticate success!")
	}

	trans.End(trans.LastStatus())
}
