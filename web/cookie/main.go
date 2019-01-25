package main

import (
	"encoding/json"
	"fmt"
	"github.com/gin-gonic/gin"
	"io/ioutil"
	"net/http"
	"time"
)

func main() {
	var eng = gin.Default()
	eng.GET("/cookie", handleCookie)

	go func() {
		time.Sleep(time.Second * 5)
		sendGetRequest()
	}()
	eng.Run(":8087")
}

func handleCookie(c *gin.Context) {
	fmt.Println("[Server] [Cookie] join handler")
	for _, cookie := range c.Request.Cookies() {
		fmt.Println("[Server] Name: ", cookie.Name,
			"\n\tDetail:", jsonString(cookie))
		cookie.Value = "jouyouyunTest"
		cookie.Domain = "jouyouyun.top"
		cookie.MaxAge = 3600
		http.SetCookie(c.Writer, cookie)
	}
	v, err := c.Cookie("login")
	if err != nil {
		fmt.Println("[Server] Failed to found cookie for login:", err)
		c.JSON(http.StatusBadRequest, gin.H{
			"error": "not found login cookie",
		})
		return
	}
	c.JSON(http.StatusOK, gin.H{
		"login": v,
	})
}

func sendGetRequest() {
	req, err := http.NewRequest(http.MethodGet,
		"http://127.0.0.1:8087/cookie", nil)
	if err != nil {
		fmt.Println("[Client] Failed to build get cookie request:", err)
		return
	}
	cookie := http.Cookie{
		Name:   "login",
		Value:  "jouyouyun",
		Path:   "/",
		Domain: "jouyouyun.top",
		MaxAge: 3600,
		Raw:    "login=jouyouyun; Domain=jouyouyun.top; Max-Age=3600",
	}
	req.AddCookie(&cookie)
	var cli = new(http.Client)
	resp, err := cli.Do(req)
	if err != nil {
		fmt.Println("[Client] Failed to send get cookie request:", err)
		return
	}
	defer resp.Body.Close()
	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("[Client] Failed to read get cookie response:", err)
		return
	}
	fmt.Printf("[Client] Response: [%d] - %s\n", resp.StatusCode, string(data))
	for _, c := range resp.Cookies() {
		fmt.Println("[Client] Name: ", c.Name,
			"\n\tDetail:", jsonString(c))
	}
}

func jsonString(v interface{}) string {
	data, _ := json.Marshal(v)
	return string(data)
}
