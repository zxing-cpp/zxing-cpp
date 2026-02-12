/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include <stdlib.h>
#include "ZXingC.h"
*/
import "C"

import (
	"fmt"
	"unsafe"
)

func (f BarcodeFormat) Symbology() BarcodeFormat {
	return BarcodeFormat(C.ZXing_BarcodeFormatSymbology(C.ZXing_BarcodeFormat(f)))
}

func (f BarcodeFormat) String() string {
	ptr := C.ZXing_BarcodeFormatToString(C.ZXing_BarcodeFormat(f))
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}

func ParseBarcodeFormat(s string) (BarcodeFormat, error) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))

	fmtVal := BarcodeFormat(C.ZXing_BarcodeFormatFromString(cs))
	if fmtVal == BarcodeFormatInvalid {
		return BarcodeFormatInvalid, errorFromLast(fmt.Sprintf("invalid barcode format: %q", s))
	}
	return fmtVal, nil
}

func ListBarcodeFormats(filter BarcodeFormat) ([]BarcodeFormat, error) {
	var count C.int
	ptr := C.ZXing_BarcodeFormatsList(C.ZXing_BarcodeFormat(filter), &count)
	if ptr == nil {
		return nil, errorFromLast("failed to list barcode formats")
	}
	defer freeZXing(unsafe.Pointer(ptr))

	n := int(count)
	if n <= 0 {
		return nil, nil
	}
	// ZXing_BarcodeFormat is an enum; treat it as 32-bit.
	src := unsafe.Slice((*C.ZXing_BarcodeFormat)(unsafe.Pointer(ptr)), n)
	out := make([]BarcodeFormat, n)
	for i := 0; i < n; i++ {
		out[i] = BarcodeFormat(src[i])
	}
	return out, nil
}

func ParseBarcodeFormats(s string) ([]BarcodeFormat, error) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))

	var count C.int
	ptr := C.ZXing_BarcodeFormatsFromString(cs, &count)
	if ptr == nil {
		return nil, errorFromLast(fmt.Sprintf("invalid barcode format set: %q", s))
	}
	defer freeZXing(unsafe.Pointer(ptr))

	n := int(count)
	if n <= 0 {
		return nil, nil
	}
	src := unsafe.Slice((*C.ZXing_BarcodeFormat)(unsafe.Pointer(ptr)), n)
	out := make([]BarcodeFormat, n)
	for i := 0; i < n; i++ {
		out[i] = BarcodeFormat(src[i])
	}
	return out, nil
}

func BarcodeFormatsString(formats []BarcodeFormat) (string, error) {
	if len(formats) == 0 {
		return "", nil
	}
	// Build a temporary C array on the Go heap; it is only used during the call.
	// (The C API copies the values and never stores the pointer.)
	tmp := make([]C.ZXing_BarcodeFormat, len(formats))
	for i, f := range formats {
		tmp[i] = C.ZXing_BarcodeFormat(f)
	}
	ptr := C.ZXing_BarcodeFormatsToString(&tmp[0], C.int(len(tmp)))
	if ptr == nil {
		return "", errorFromLast("failed to stringify barcode formats")
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s, nil
}
