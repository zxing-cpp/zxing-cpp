/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include "ZXingC.h"
*/
import "C"

import (
	"fmt"
	"image"
	"runtime"
	"unsafe"
)

type ReaderOptions struct {
	ptr *C.ZXing_ReaderOptions
}

func NewReaderOptions() (*ReaderOptions, error) {
	p := C.ZXing_ReaderOptions_new()
	if p == nil {
		return nil, errorFromLast("failed to create ReaderOptions")
	}
	o := &ReaderOptions{ptr: p}
	setFinalizer(o, (*ReaderOptions).finalize)
	return o, nil
}

func (o *ReaderOptions) finalize() { _ = o.Close() }

func (o *ReaderOptions) Close() error {
	if o == nil {
		return nil
	}
	if o.ptr != nil {
		C.ZXing_ReaderOptions_delete(o.ptr)
		o.ptr = nil
	}
	runtime.KeepAlive(o)
	return nil
}

func (o *ReaderOptions) SetTryHarder(v bool) { C.ZXing_ReaderOptions_setTryHarder(o.ptr, C.bool(v)) }
func (o *ReaderOptions) TryHarder() bool     { return bool(C.ZXing_ReaderOptions_getTryHarder(o.ptr)) }

func (o *ReaderOptions) SetTryRotate(v bool) { C.ZXing_ReaderOptions_setTryRotate(o.ptr, C.bool(v)) }
func (o *ReaderOptions) TryRotate() bool     { return bool(C.ZXing_ReaderOptions_getTryRotate(o.ptr)) }

func (o *ReaderOptions) SetTryInvert(v bool) { C.ZXing_ReaderOptions_setTryInvert(o.ptr, C.bool(v)) }
func (o *ReaderOptions) TryInvert() bool     { return bool(C.ZXing_ReaderOptions_getTryInvert(o.ptr)) }

func (o *ReaderOptions) SetTryDownscale(v bool) { C.ZXing_ReaderOptions_setTryDownscale(o.ptr, C.bool(v)) }
func (o *ReaderOptions) TryDownscale() bool     { return bool(C.ZXing_ReaderOptions_getTryDownscale(o.ptr)) }

func (o *ReaderOptions) SetIsPure(v bool) { C.ZXing_ReaderOptions_setIsPure(o.ptr, C.bool(v)) }
func (o *ReaderOptions) IsPure() bool     { return bool(C.ZXing_ReaderOptions_getIsPure(o.ptr)) }

func (o *ReaderOptions) SetValidateOptionalChecksum(v bool) {
	C.ZXing_ReaderOptions_setValidateOptionalChecksum(o.ptr, C.bool(v))
}
func (o *ReaderOptions) ValidateOptionalChecksum() bool {
	return bool(C.ZXing_ReaderOptions_getValidateOptionalChecksum(o.ptr))
}

func (o *ReaderOptions) SetReturnErrors(v bool) { C.ZXing_ReaderOptions_setReturnErrors(o.ptr, C.bool(v)) }
func (o *ReaderOptions) ReturnErrors() bool     { return bool(C.ZXing_ReaderOptions_getReturnErrors(o.ptr)) }

func (o *ReaderOptions) SetMinLineCount(n int) { C.ZXing_ReaderOptions_setMinLineCount(o.ptr, C.int(n)) }
func (o *ReaderOptions) MinLineCount() int     { return int(C.ZXing_ReaderOptions_getMinLineCount(o.ptr)) }

func (o *ReaderOptions) SetMaxNumberOfSymbols(n int) { C.ZXing_ReaderOptions_setMaxNumberOfSymbols(o.ptr, C.int(n)) }
func (o *ReaderOptions) MaxNumberOfSymbols() int     { return int(C.ZXing_ReaderOptions_getMaxNumberOfSymbols(o.ptr)) }

func (o *ReaderOptions) SetBinarizer(b Binarizer) {
	C.ZXing_ReaderOptions_setBinarizer(o.ptr, C.ZXing_Binarizer(b))
}
func (o *ReaderOptions) Binarizer() Binarizer {
	return Binarizer(C.ZXing_ReaderOptions_getBinarizer(o.ptr))
}

func (o *ReaderOptions) SetEanAddOnSymbol(v EanAddOnSymbol) {
	C.ZXing_ReaderOptions_setEanAddOnSymbol(o.ptr, C.ZXing_EanAddOnSymbol(v))
}
func (o *ReaderOptions) EanAddOnSymbol() EanAddOnSymbol {
	return EanAddOnSymbol(C.ZXing_ReaderOptions_getEanAddOnSymbol(o.ptr))
}

func (o *ReaderOptions) SetTextMode(v TextMode) { C.ZXing_ReaderOptions_setTextMode(o.ptr, C.ZXing_TextMode(v)) }
func (o *ReaderOptions) TextMode() TextMode     { return TextMode(C.ZXing_ReaderOptions_getTextMode(o.ptr)) }

func (o *ReaderOptions) SetFormats(formats []BarcodeFormat) {
	if len(formats) == 0 {
		return
	}
	tmp := make([]C.ZXing_BarcodeFormat, len(formats))
	for i, f := range formats {
		tmp[i] = C.ZXing_BarcodeFormat(f)
	}
	C.ZXing_ReaderOptions_setFormats(o.ptr, &tmp[0], C.int(len(tmp)))
	runtime.KeepAlive(tmp)
}

func (o *ReaderOptions) Formats() ([]BarcodeFormat, error) {
	var count C.int
	ptr := C.ZXing_ReaderOptions_getFormats(o.ptr, &count)
	if ptr == nil {
		// Not an error: it can be empty.
		if msg, ok := lastErrorString(); ok {
			return nil, fmt.Errorf("failed to get formats: %s", msg)
		}
		return nil, nil
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

func readBarcodesFromImageView(iv *ImageView, opts *ReaderOptions) ([]*Barcode, error) {
	if iv == nil || iv.ptr == nil {
		return nil, fmt.Errorf("image view is nil")
	}
	var optsPtr *C.ZXing_ReaderOptions
	if opts != nil {
		optsPtr = opts.ptr
	}

	res := C.ZXing_ReadBarcodes(iv.ptr, optsPtr)
	if res == nil {
		return nil, errorFromLast("failed to read barcodes")
	}
	defer C.ZXing_Barcodes_delete(res)

	size := int(C.ZXing_Barcodes_size(res))
	if size <= 0 {
		return nil, nil
	}
	out := make([]*Barcode, 0, size)
	for i := 0; i < size; i++ {
		bc := C.ZXing_Barcodes_move(res, C.int(i))
		if bc == nil {
			return nil, errorFromLast("failed to move barcode")
		}
		b := &Barcode{ptr: bc}
		setFinalizer(b, (*Barcode).finalize)
		out = append(out, b)
	}
	return out, nil
}

func readBarcodesFromImage(img image.Image, opts *ReaderOptions) ([]*Barcode, error) {
	iv, err := NewImageViewFromImage(img)
	if err != nil {
		return nil, err
	}
	defer iv.Close()
	return readBarcodesFromImageView(iv, opts)
}

// ReadBarcodes scans for barcodes from either a *ImageView or a standard image.Image.
//
// Supported inputs:
//   - *ImageView
//   - image.Image (implicitly converted via NewImageViewFromImage)
//
// The opts parameter is optional. Call ReadBarcodes(src) for defaults,
// or ReadBarcodes(src, opts) to provide ReaderOptions.
func ReadBarcodes(src any, opts ...*ReaderOptions) ([]*Barcode, error) {
	var ro *ReaderOptions
	if len(opts) > 0 {
		ro = opts[0]
	}

	switch v := src.(type) {
	case *ImageView:
		return readBarcodesFromImageView(v, ro)
	case image.Image:
		return readBarcodesFromImage(v, ro)
	default:
		return nil, fmt.Errorf("unsupported source type %T; expected *ImageView or image.Image", src)
	}
}
