package main

import (
	"context"
	"flag"
	"fmt"
	"time"

	"github.com/varlink/go/varlink"
)

type Calculator struct {
	VarlinkInterface
	vendor  string
	product string
	version string
	url     string
	address string
}

func (calc *Calculator) Add(ctx context.Context, call VarlinkCall, i, j int64) error {
	time.Sleep(time.Second * 5)
	return call.ReplyAdd(ctx, i+j+a)
}

func (calc *Calculator) Sub(ctx context.Context, call VarlinkCall, i, j int64) error {
	return call.ReplySub(ctx, i-j)
}

func (calc *Calculator) Multiple(ctx context.Context, call VarlinkCall, i, j int64) error {
	return call.ReplySub(ctx, i*j)
}

func (calc *Calculator) Division(ctx context.Context, call VarlinkCall, i, j int64) error {
	if j == 0 {
		return call.ReplyInvalidParameter(ctx, "j")
	}
	return call.ReplySub(ctx, i/j)
}

func main() {
	var (
		_vendor  = flag.String("vendor", "Jouyouyun", "The varlink service vendor")
		_product = flag.String("product", "Calculator", "The varlink service product")
		_version = flag.String("version", "1", "The varlink service version")
		_url     = flag.String("url", "https://jouyouyun.github.io", "The varlink service url")
		_address = flag.String("address", "", "The varlink service address")
	)

	flag.Parse()
	calc := Calculator{
		vendor:  *_vendor,
		product: *_product,
		version: *_version,
		url:     *_url,
		address: *_address,
	}

	srv, err := varlink.NewService(calc.vendor, calc.product, calc.vendor, calc.url)
	if err != nil {
		fmt.Printf("varlink service new service failed: %+v\n", err)
		return
	}

	err = srv.RegisterInterface(VarlinkNew(&calc))
	if err != nil {
		fmt.Printf("varlink service register failed: %+v\n", err)
		return
	}

	err = srv.Listen(context.Background(), calc.address, 0)
	if err != nil {
		fmt.Printf("varlink service listen failed: %+v\n", err)
		return
	}
}
