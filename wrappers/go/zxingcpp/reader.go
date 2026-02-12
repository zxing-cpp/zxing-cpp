// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include "zxingcgo.h"
*/
import "C"

import (
	"fmt"
	"image"
	"runtime"
	"unsafe"
)

// ReaderOption is a functional option for configuring reader options.
type ReaderOption func(*C.ZXing_ReaderOptions)

// TryHarder instructs the reader to prioritize accuracy over speed.
func TryHarder(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setTryHarder(o, C.bool(v)) }
}

// TryRotate enables detection in rotated images (90, 180, 270 degrees).
func TryRotate(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setTryRotate(o, C.bool(v)) }
}

// TryInvert enables detection of inverted (reversed reflectance) codes when supported.
func TryInvert(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setTryInvert(o, C.bool(v)) }
}

// TryDownscale enables detection in downscaled images depending on image size.
func TryDownscale(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setTryDownscale(o, C.bool(v)) }
}

// IsPure indicates that the input contains a single perfectly aligned barcode (generated image).
func IsPure(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setIsPure(o, C.bool(v)) }
}

// ValidateOptionalChecksum enables validation of optional checksums where applicable (e.g. Code39, ITF).
func ValidateOptionalChecksum(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setValidateOptionalChecksum(o, C.bool(v)) }
}

// ReturnErrors makes ReadBarcodes return barcodes with errors (e.g. checksum errors) when true.
func ReturnErrors(v bool) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setReturnErrors(o, C.bool(v)) }
}

// WithMinLineCount sets the number of scan lines in a linear barcode that must agree to accept the result.
func WithMinLineCount(n int) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setMinLineCount(o, C.int(n)) }
}

// WithMaxNumberOfSymbols sets the maximum number of symbols (barcodes) to detect.
func WithMaxNumberOfSymbols(n int) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setMaxNumberOfSymbols(o, C.int(n)) }
}

// WithBinarizer selects the binarizer used for grayscale-to-binary conversion.
func WithBinarizer(v Binarizer) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setBinarizer(o, C.ZXing_Binarizer(v)) }
}

// WithEanAddOnSymbol controls how EAN-2/5 add-on symbols are handled (ignore/read/require).
func WithEanAddOnSymbol(v EanAddOnSymbol) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setEanAddOnSymbol(o, C.ZXing_EanAddOnSymbol(v)) }
}

// WithTextMode sets the TextMode that controls the output of Barcode.text().
func WithTextMode(v TextMode) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) { C.ZXing_ReaderOptions_setTextMode(o, C.ZXing_TextMode(v)) }
}

// WithFormats restricts the set of BarcodeFormats to search for (default: all supported formats).
func WithFormats(formats ...BarcodeFormat) ReaderOption {
	return func(o *C.ZXing_ReaderOptions) {
		ptr := (*C.ZXing_BarcodeFormat)(unsafe.Pointer(unsafe.SliceData(formats)))
		C.ZXing_ReaderOptions_setFormats(o, ptr, C.int(len(formats)))
		runtime.KeepAlive(formats)
	}
}

func readBarcodesFromImageView(iv *ImageView, opts *C.ZXing_ReaderOptions) ([]*Barcode, error) {
	if iv == nil || iv.ptr == nil {
		return nil, fmt.Errorf("iv is nil")
	}

	res := C.ZXing_ReadBarcodes(iv.ptr, opts)
	if res == nil {
		return nil, zxLastError()
	}

	size := int(C.ZXing_Barcodes_size(res))
	if size <= 0 {
		return nil, nil
	}

	defer C.ZXing_Barcodes_delete(res)

	out := make([]*Barcode, 0, size)
	for i := range size {
		bc := C.ZXing_Barcodes_move(res, C.int(i))
		if bc == nil {
			return nil, zxLastError("failed to move barcode")
		}
		out = append(out, &Barcode{ptr: bc})
	}
	return out, nil
}

// ReadBarcodes scans for barcodes from either a *ImageView or a standard image.Image with optional configuration options,
// returning a slice of found barcodes. Example usage:
//
//	barcodes, err := ReadBarcodes(img, WithFormats(BarcodeFormatQRCode), TryInvert(false))
func ReadBarcodes(src any, opts ...ReaderOption) ([]*Barcode, error) {
	var ro *C.ZXing_ReaderOptions
	if len(opts) > 0 {
		ro = C.ZXing_ReaderOptions_new()
		if ro == nil {
			panic("failed to create ReaderOptions") // only out of memory should cause this, so panic is reasonable
		}
		defer C.ZXing_ReaderOptions_delete(ro)

		for _, opt := range opts {
			opt(ro)
		}
	}

	switch v := src.(type) {
	case *ImageView:
		return readBarcodesFromImageView(v, ro)
	case image.Image:
		iv, err := NewImageViewFromImage(v)
		if err != nil {
			return nil, err
		}
		defer iv.Close()
		return readBarcodesFromImageView(iv, ro)
	default:
		return nil, fmt.Errorf("unsupported source type %T; expected *ImageView or image.Image", src)
	}
}
