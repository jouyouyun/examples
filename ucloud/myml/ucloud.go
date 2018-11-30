package main

import (
	"context"
	"fmt"
	"github.com/google/go-cloud/blob"
	"io"
	"io/ioutil"
	"time"
)

var (
	defaultTimeout = time.Second * 30
)

func Get(key string) ([]byte, error) {
	fmt.Println("[Get] key:", key)
	ctx, cancel := context.WithTimeout(context.Background(), defaultTimeout)
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
	var opts = blob.ListOptions{}
	opts.Prefix = dir
	iter := bucket.List(&opts)
	if iter == nil {
		return nil, fmt.Errorf("no item found")
	}

	var ret []string
	for {
		ctx, cancel := context.WithTimeout(context.Background(), defaultTimeout)
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
	ctx, cancel := context.WithTimeout(context.Background(), defaultTimeout)
	defer cancel()
	return bucket.Delete(ctx, key)
}

// Size return the key size(byte), if key is dir, will recursion
func Size(key string) (uint64, error) {
	var iter = bucket.List(&blob.ListOptions{Prefix: key})
	var size uint64
	for {
		ctx, cancel := context.WithTimeout(context.Background(), defaultTimeout)
		item, err := iter.Next(ctx)
		cancel()
		if err != nil {
			if err == io.EOF {
				break
			}
			return 0, err
		}
		if item.Size != 0 {
			size += uint64(item.Size)
			continue
		}
		// if size == 0 maybe is dir in upyun
		s, _ := Size(item.Key + "/")
		size += s
	}
	return size, nil
}
