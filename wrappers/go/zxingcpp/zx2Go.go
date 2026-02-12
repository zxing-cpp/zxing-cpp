// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include "zxingcgo.h"
*/
import "C"

import (
	"errors"
	"unsafe"
)

func zxString2Go(ptr *C.char) string {
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	C.ZXing_free(unsafe.Pointer(ptr))
	return s
}

func zxBytes2Go(f func(n *C.int) *C.uint8_t) []byte {
	var n C.int
	ptr := f(&n)
	if ptr == nil {
		return nil
	}
	defer C.ZXing_free(unsafe.Pointer(ptr))
	if n <= 0 {
		return nil
	}
	return C.GoBytes(unsafe.Pointer(ptr), n)
}

func zxBarcodeFormats2Go(ptr *C.ZXing_BarcodeFormat, count C.int) []BarcodeFormat {
	defer C.ZXing_free(unsafe.Pointer(ptr))
	if ptr == nil || count <= 0 {
		return nil
	}
	return append([]BarcodeFormat(nil), unsafe.Slice((*BarcodeFormat)(unsafe.Pointer(ptr)), count)...)
}

func zxLastError(defaultMsg ...string) error {
	ptr := C.ZXing_LastErrorMsg()
	if ptr != nil {
		return errors.New(zxString2Go(ptr))
	} else if len(defaultMsg) > 0 {
		return errors.New(defaultMsg[0])
	} else {
		return nil
	}
}
