package main

import (
	"context"
	"fmt"
	"github.com/google/go-cloud/blob"
	"io"
	"io/ioutil"
	"time"
)

func Get(key string) ([]byte, error) {
	fmt.Println("[Get] key:", key)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*30)
	defer cancel()
	reader, err := bucket.NewReader(ctx, key)
	if err != nil {
		return nil, err
	}
	defer reader.Close()
	return ioutil.ReadAll(reader)
}

func List(dir string) ([]string, error) {
	fmt.Println("[List] dir:", dir)
	var opts = blob.ListOptions{Prefix: dir}
	var iter = bucket.List(&opts)
	var ret []string
	for {
		ctx, cancel := context.WithTimeout(context.Background(), time.Second*30)
		obj, err := iter.Next(ctx)
		cancel()
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Println("Failed to list at next:", err)
			return nil, err
		}
		fmt.Println("[List] item:", obj.IsDir, obj.Key, obj.Size, obj.ModTime)
		ret = append(ret, obj.Key)
	}
	return ret, nil
}

func Delete(key string) error {
	fmt.Println("[Delete] key:", key)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*30)
	defer cancel()
	return bucket.Delete(ctx, key)
}
