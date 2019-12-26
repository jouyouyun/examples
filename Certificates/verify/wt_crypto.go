package main

// #cgo CFLAGS: -Wall -g
// #cgo LDFLAGS: -lmkpkcs11
// #include <stdlib.h>
// #include <deepin/signature.h>
import "C"

import (
	"fmt"
	"unsafe"
)

// WTCrypto wotong crypto handler
type WTCrypto struct {
	Pin       string
	Container string
	ID        string
	Algorithm uint64
}

// Init init wotong environment
func (wt *WTCrypto) Init() error {
	var config = C.wosign_sign_config{
		pin:       C.CString(wt.Pin),
		container: C.CString(wt.Container),
		id:        C.CString(wt.ID),
		algorithm: C.ulong(wt.Algorithm),
	}

	ret := C.wosign_config_init(&config)
	if int(ret) != 0 {
		return fmt.Errorf("Failed to init environment, error code: %v", int(ret))
	}

	return nil
}

// Signature signature data
func (wt *WTCrypto) Signature(data []byte) ([]byte, error) {
	len := len(data)
	cdata := C.CString(string(data))
	defer C.free(unsafe.Pointer(cdata))

	var sign *C.char
	var signLen C.ulong
	ret := C.wosign_sign(cdata, C.ulong(len), (**C.char)(&sign), &signLen)
	if int(ret) != 0 {
		return nil, fmt.Errorf("Failed to signature, error code: %v", int(ret))
	}

	return []byte(C.GoStringN(sign, C.int(signLen))), nil
}
