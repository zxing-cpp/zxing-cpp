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
	"image"
	"runtime"
	"unsafe"
)

type CreatorOptions struct {
	ptr *C.ZXing_CreatorOptions
}

func NewCreatorOptions(format BarcodeFormat) (*CreatorOptions, error) {
	p := C.ZXing_CreatorOptions_new(C.ZXing_BarcodeFormat(format))
	if p == nil {
		return nil, errorFromLast("failed to create CreatorOptions")
	}
	o := &CreatorOptions{ptr: p}
	setFinalizer(o, (*CreatorOptions).finalize)
	return o, nil
}

func (o *CreatorOptions) finalize() { _ = o.Close() }

func (o *CreatorOptions) Close() error {
	if o == nil {
		return nil
	}
	if o.ptr != nil {
		C.ZXing_CreatorOptions_delete(o.ptr)
		o.ptr = nil
	}
	runtime.KeepAlive(o)
	return nil
}

func (o *CreatorOptions) SetFormat(f BarcodeFormat) { C.ZXing_CreatorOptions_setFormat(o.ptr, C.ZXing_BarcodeFormat(f)) }
func (o *CreatorOptions) Format() BarcodeFormat     { return BarcodeFormat(C.ZXing_CreatorOptions_getFormat(o.ptr)) }

func (o *CreatorOptions) SetOptions(s string) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))
	C.ZXing_CreatorOptions_setOptions(o.ptr, cs)
}

func (o *CreatorOptions) Options() (string, error) {
	ptr := C.ZXing_CreatorOptions_getOptions(o.ptr)
	if ptr == nil {
		return "", errorFromLast("ZXing_CreatorOptions_getOptions returned NULL")
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s, nil
}

func CreateBarcodeFromText(data string, opts *CreatorOptions) (*Barcode, error) {
	if opts == nil || opts.ptr == nil {
		return nil, fmt.Errorf("creator options are nil")
	}
	// C strings cannot contain NUL. Users needing embedded NUL should use CreateBarcodeFromBytes.
	cs := C.CString(data)
	defer C.free(unsafe.Pointer(cs))

	bc := C.ZXing_CreateBarcodeFromText(cs, C.int(len(data)), opts.ptr)
	if bc == nil {
		return nil, errorFromLast("failed to create barcode from text")
	}
	b := &Barcode{ptr: bc}
	setFinalizer(b, (*Barcode).finalize)
	return b, nil
}

func CreateBarcodeFromBytes(data []byte, opts *CreatorOptions) (*Barcode, error) {
	if opts == nil || opts.ptr == nil {
		return nil, fmt.Errorf("creator options are nil")
	}
	if len(data) == 0 {
		return nil, fmt.Errorf("data is empty")
	}
	cbuf := C.CBytes(data)
	if cbuf == nil {
		return nil, fmt.Errorf("failed to allocate C memory")
	}
	defer C.free(cbuf)

	bc := C.ZXing_CreateBarcodeFromBytes(cbuf, C.int(len(data)), opts.ptr)
	if bc == nil {
		return nil, errorFromLast("failed to create barcode from bytes")
	}
	b := &Barcode{ptr: bc}
	setFinalizer(b, (*Barcode).finalize)
	return b, nil
}

type WriterOptions struct {
	ptr *C.ZXing_WriterOptions
}

func NewWriterOptions() (*WriterOptions, error) {
	p := C.ZXing_WriterOptions_new()
	if p == nil {
		return nil, errorFromLast("failed to create WriterOptions")
	}
	o := &WriterOptions{ptr: p}
	setFinalizer(o, (*WriterOptions).finalize)
	return o, nil
}

func (o *WriterOptions) finalize() { _ = o.Close() }

func (o *WriterOptions) Close() error {
	if o == nil {
		return nil
	}
	if o.ptr != nil {
		C.ZXing_WriterOptions_delete(o.ptr)
		o.ptr = nil
	}
	runtime.KeepAlive(o)
	return nil
}

func (o *WriterOptions) SetScale(v int)  { C.ZXing_WriterOptions_setScale(o.ptr, C.int(v)) }
func (o *WriterOptions) Scale() int      { return int(C.ZXing_WriterOptions_getScale(o.ptr)) }
func (o *WriterOptions) SetRotate(v int) { C.ZXing_WriterOptions_setRotate(o.ptr, C.int(v)) }
func (o *WriterOptions) Rotate() int     { return int(C.ZXing_WriterOptions_getRotate(o.ptr)) }

func (o *WriterOptions) SetAddHRT(v bool) { C.ZXing_WriterOptions_setAddHRT(o.ptr, C.bool(v)) }
func (o *WriterOptions) AddHRT() bool     { return bool(C.ZXing_WriterOptions_getAddHRT(o.ptr)) }

func (o *WriterOptions) SetAddQuietZones(v bool) {
	C.ZXing_WriterOptions_setAddQuietZones(o.ptr, C.bool(v))
}
func (o *WriterOptions) AddQuietZones() bool {
	return bool(C.ZXing_WriterOptions_getAddQuietZones(o.ptr))
}

func (b *Barcode) ToSVG(opts *WriterOptions) (string, error) {
	if b == nil || b.ptr == nil {
		return "", fmt.Errorf("barcode is nil")
	}
	var p *C.ZXing_WriterOptions
	if opts != nil {
		p = opts.ptr
	}
	s := C.ZXing_WriteBarcodeToSVG(b.ptr, p)
	if s == nil {
		return "", errorFromLast("failed to write barcode to SVG")
	}
	out := C.GoString(s)
	freeZXing(unsafe.Pointer(s))
	return out, nil
}

func (b *Barcode) ToImage(opts *WriterOptions) (image.Image, error) {
	if b == nil || b.ptr == nil {
		return nil, fmt.Errorf("barcode is nil")
	}
	var p *C.ZXing_WriterOptions
	if opts != nil {
		p = opts.ptr
	}
	return toGoImageAndFree(C.ZXing_WriteBarcodeToImage(b.ptr, p))
}
