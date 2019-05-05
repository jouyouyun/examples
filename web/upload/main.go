package main

import (
	"crypto/md5"
	"fmt"
	"io"
	"path/filepath"

	"mime/multipart"

	"os"

	"github.com/gin-gonic/gin"
)

func main() {
	var engine = gin.Default()

	// curl -X POST <url>/single -F "file=@<filepath>"
	engine.POST("/single", uploadSingleFile)
	// curl -X POST <url>/multi -F "files=@<filepath1>"-F "files=@<filepath2>"
	engine.POST("/multi", uploadMultiFiles)

	engine.Run(":8005")
}

func uploadSingleFile(c *gin.Context) {
	file, err := c.FormFile("file")
	if err != nil {
		c.JSON(400, gin.H{
			"error": err.Error(),
		})
		return
	}
	err = saveFile(c, file)
	if err != nil {
		c.JSON(500, gin.H{
			"error": err.Error(),
		})
		return
	}
}

func uploadMultiFiles(c *gin.Context) {
	form, err := c.MultipartForm()
	if err != nil {
		c.JSON(400, gin.H{
			"error": err.Error(),
		})
		return
	}
	files, ok := form.File["files"]
	if !ok {
		c.JSON(400, gin.H{
			"error": "no files exists",
		})
		return
	}

	for _, file := range files {
		err = saveFile(c, file)
		if err != nil {
			c.JSON(500, gin.H{
				"error": err.Error(),
			})
			return
		}
	}
}

func saveFile(c *gin.Context, file *multipart.FileHeader) error {
	fp, err := file.Open()
	if err != nil {
		return err
	}
	defer fp.Close()

	h := md5.New()
	io.Copy(h, fp)
	key := fmt.Sprintf("%x", h.Sum(nil))

	dest := filepath.Join("/tmp", key+filepath.Ext(file.Filename))
	if _, err := os.Stat(dest); err == nil || os.IsExist(err) {
		return nil
	}

	err = c.SaveUploadedFile(file, dest)
	if err != nil {
		return err
	}

	return nil
}
