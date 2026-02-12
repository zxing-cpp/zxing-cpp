// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include <stdlib.h>
#include "zxingcgo.h"
*/
import "C"

import (
	"runtime"
	"unsafe"
)

// Symbology returns the base symbology for the given BarcodeFormat.
func (f BarcodeFormat) Symbology() BarcodeFormat {
	return BarcodeFormat(C.ZXing_BarcodeFormatSymbology(C.ZXing_BarcodeFormat(f)))
}

// String returns a human-readable name for the BarcodeFormat.
func (f BarcodeFormat) String() string {
	return zxString2Go(C.ZXing_BarcodeFormatToString(C.ZXing_BarcodeFormat(f)))
}

// ParseBarcodeFormat parses a string into a BarcodeFormat. Returns an error if parsing fails.
func ParseBarcodeFormat(s string) (BarcodeFormat, error) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))

	fmtVal := BarcodeFormat(C.ZXing_BarcodeFormatFromString(cs))
	if fmtVal == BarcodeFormatInvalid {
		return BarcodeFormatInvalid, zxLastError()
	}
	return fmtVal, nil
}

// ListBarcodeFormats returns a slice of BarcodeFormat values filtered by the provided mask.
func ListBarcodeFormats(filter BarcodeFormat) []BarcodeFormat {
	var count C.int
	ptr := C.ZXing_BarcodeFormatsList(C.ZXing_BarcodeFormat(filter), &count)
	if ptr == nil {
		panic("ZXing_BarcodeFormatsList failed")
	}
	return zxBarcodeFormats2Go(ptr, count)
}

// ParseBarcodeFormats parses a comma-separated list of format names and returns the corresponding BarcodeFormats.
func ParseBarcodeFormats(s string) ([]BarcodeFormat, error) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))

	var count C.int
	ptr := C.ZXing_BarcodeFormatsFromString(cs, &count)
	if ptr == nil {
		return nil, zxLastError()
	}
	return zxBarcodeFormats2Go(ptr, count), nil
}

// BarcodeFormatsString returns a human-readable string representation of the provided BarcodeFormats.
func BarcodeFormatsString(formats []BarcodeFormat) string {
	if len(formats) == 0 {
		return ""
	}
	ptr := C.ZXing_BarcodeFormatsToString(
		(*C.ZXing_BarcodeFormat)(unsafe.Pointer(unsafe.SliceData(formats))),
		C.int(len(formats)),
	)
	runtime.KeepAlive(formats)
	if ptr == nil {
		panic("ZXing_BarcodeFormatsToString failed")
	}
	return zxString2Go(ptr)
}
