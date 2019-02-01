package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"github.com/gin-gonic/gin"
	"io/ioutil"
	"net/http"
	"pkg.deepin.io/server/utils/logger"
)

type githubOAuthService struct {
	Nickname    string `json:"-"`
	AccessToken string `json:"access_token"`
	OpenID      string `json:"open_id"`
	Error       string `json:"error"`
	ErrorDesc   string `json:"error_description"`
}

const (
	githubServiceLoginAPI      = "https://github.com/login/oauth"
	githubServiceLoginAPIAuth  = githubServiceLoginAPI + "/authorize"
	githubServiceLoginAPIToken = githubServiceLoginAPI + "/access_token"
	githubServiceAPI           = "https://api.github.com"
	githubServiceAPIUser       = githubServiceAPI + "/user"
)

var (
	clientID     = flag.String("id", "", "Github Client ID")
	clientSecret = flag.String("secret", "", "Github Client Secret")
)

func main() {
	flag.Parse()
	if len(*clientID) == 0 || len(*clientSecret) == 0 {
		flag.Usage()
		return
	}
	var eng = gin.Default()
	eng.GET("/code/:code", func(c *gin.Context) {
		git, err := getUserToken(c.Param("code"))
		if err != nil {
			c.String(404, err.Error())
			return
		}
		err = getUserInfo(git)
		if err != nil {
			c.String(404, err.Error())
			return
		}
	})
	eng.Run(":7112")
}

func getUserToken(code string) (*githubOAuthService, error) {
	req, err := http.NewRequest(http.MethodGet, githubServiceLoginAPIToken, nil)
	if err != nil {
		logger.Warning("Failed to build github token request:", err)
		return nil, err
	}
	params := req.URL.Query()
	//redirectURI := "http://test.api.deepinid.deepin.com/v1/tpl/callback/login/github"
	params.Add("code", code)
	params.Add("client_id", *clientID)
	params.Add("client_secret", *clientSecret)
	//params.Add("redirect_uri", redirectURI)
	req.URL.RawQuery = params.Encode()
	fmt.Println("Request url:", req.URL.String())
	req.Header.Set("Accept", "application/json")

	client := new(http.Client)
	resp, err := client.Do(req)
	if err != nil {
		logger.Warning("Failed to send github token request:", err)
		return nil, err
	}
	defer resp.Body.Close()
	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		logger.Warning("Failed to read response:", string(data))
		return nil, err
	}

	var git githubOAuthService
	err = json.Unmarshal(data, &git)
	if err != nil {
		logger.Warning("Failed to parse github token response:", err, string(data))
		return nil, err
	}
	if len(git.Error) != 0 {
		logger.Warning("Failed to get github token:", git.ErrorDesc)
		return nil, fmt.Errorf(git.Error)
	}
	return &git, nil
}

func getUserInfo(git *githubOAuthService) error {
	req, err := http.NewRequest(http.MethodGet, githubServiceAPIUser, nil)
	if err != nil {
		logger.Warning("Failed to build github user info request:", err)
		return err
	}
	req.Header.Set("Authorization", "token "+git.AccessToken)
	client := new(http.Client)
	resp, err := client.Do(req)
	if err != nil {
		logger.Warning("Failed to send github user info request:", err)
		return err
	}
	defer resp.Body.Close()
	var userInfo = struct {
		Name   string `json:"login"`
		Avatar string `json:"avatar_url"`
		Error  string `json:"message"`
		UID    int64  `json:"id"`
	}{}
	err = json.NewDecoder(resp.Body).Decode(&userInfo)
	if err != nil {
		logger.Warning("Failed to parse github user info response:", err)
		return err
	}
	if len(userInfo.Error) != 0 {
		logger.Error("Failed to get github user info:", userInfo.Error)
		return fmt.Errorf(userInfo.Error)
	}
	git.Nickname = userInfo.Name
	logger.Info("User:", userInfo)
	return nil
}
