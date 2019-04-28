package router

import (
	"github-statistic/internal/log"
	"github-statistic/pkg/github"
	"github.com/gin-gonic/gin"
	"io/ioutil"
	"net/http"
)

func Route() {
	var engine = gin.Default()
	engine.POST("/v1/events", func(c *gin.Context) {
		log.Info("Header: ", c.Request.Header)
		if c.Request.Body == nil {
			c.JSON(http.StatusBadRequest, gin.H{
				"error": "invalid body",
			})
			return
		}
		data, err := ioutil.ReadAll(c.Request.Body)
		if err != nil {
			c.JSON(http.StatusBadRequest, gin.H{
				"error": err.Error(),
			})
			return
		}
		err = github.HandleEvent(c.GetHeader("X-GitHub-Event"), data)
		if err != nil {
			c.JSON(http.StatusBadRequest, gin.H{
				"error": err.Error(),
			})
			return
		}
	})

	engine.Run(":18001")
}
