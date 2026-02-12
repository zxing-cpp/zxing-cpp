// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include <stdlib.h>
#include "zxingcgo.h"
*/
import "C"

import (
	"fmt"
	"image"
	"runtime"
	"strings"
	"unsafe"
)

// WithEcLevel sets the error-correction level (e.g. "30%"). See libzint docs for supported values per symbology.
func WithEcLevel(v string) string { return "EcLevel=" + v }

// WithECI specifies the ECI designator to use (e.g. "UTF-8" or "26"). See libzint docs for details.
func WithECI(v string) string { return "ECI=" + v }

// WithGS1 enables GS1 mode (most 2D symbologies and Code128).
func WithGS1(v bool) string { return "GS1=" + fmt.Sprint(v) }

// WithReaderInit sets the reader init flag (most 2D symbologies).
func WithReaderInit(v bool) string { return "ReaderInit=" + fmt.Sprint(v) }

// WithForceSquare restricts DataMatrix to square symbol versions only.
func WithForceSquare(v bool) string { return "ForceSquare=" + fmt.Sprint(v) }

// WithColumns sets the number of columns (e.g. for DataBarExpStk, PDF417).
func WithColumns(v int) string { return "Columns=" + fmt.Sprint(v) }

// WithRows sets the number of rows (e.g. for DataBarExpStk, PDF417).
func WithRows(v int) string { return "Rows=" + fmt.Sprint(v) }

// WithVersion sets the version/size of the symbol (most 2D symbologies).
func WithVersion(v string) string { return "Version=" + v }

// WithDataMask sets the dataMask to use (QRCode/MicroQRCode).
func WithDataMask(v int) string { return "DataMask=" + fmt.Sprint(v) }

// CreateBarcode creates a barcode for the given text or byte data and format, with optional configuration options.
// The available options are symbology specific, see the With* functions above. Example usage:
//
//	bc, err := CreateBarcode("Hello, world!", BarcodeFormatQRCode, WithEcLevel("H"))
func CreateBarcode(src any, format BarcodeFormat, options ...string) (*Barcode, error) {
	opts := C.ZXing_CreatorOptions_new(C.ZXing_BarcodeFormat(format))
	if opts == nil {
		panic("ZXing_CreatorOptions_new failed") // only out of memory should cause this, so panic is reasonable
	}
	defer C.ZXing_CreatorOptions_delete(opts)

	if len(options) > 0 {
		cs := C.CString(strings.Join(options, ","))
		defer C.free(unsafe.Pointer(cs))
		C.ZXing_CreatorOptions_setOptions(opts, cs)
	}

	var bc *C.ZXing_Barcode
	switch data := src.(type) {
	case string:
		bc = C.ZXing_CreateBarcodeFromText((*C.char)(unsafe.Pointer(unsafe.StringData(data))), C.int(len(data)), opts)
		runtime.KeepAlive(data)
	case []byte:
		bc = C.ZXing_CreateBarcodeFromBytes(unsafe.Pointer(unsafe.SliceData(data)), C.int(len(data)), opts)
		runtime.KeepAlive(data)
	default:
		return nil, fmt.Errorf("unsupported source type %T; expected string or []byte", src)
	}

	if bc == nil {
		return nil, zxLastError()
	}
	return &Barcode{ptr: bc}, nil
}

// WriterOption is a function that configures the barcode writing process.
type WriterOption func(*C.ZXing_WriterOptions)

// WithScale sets the scale factor for the output image.
func WithScale(v int) WriterOption {
	return func(o *C.ZXing_WriterOptions) { C.ZXing_WriterOptions_setScale(o, C.int(v)) }
}

// WithRotation sets the rotation angle (0, 90, 180, or 270 degrees).
func WithRotation(v int) WriterOption {
	return func(o *C.ZXing_WriterOptions) { C.ZXing_WriterOptions_setRotate(o, C.int(v)) }
}

// WithHRT sets whether to add Human Readable Text.
func WithHRT(v bool) WriterOption {
	return func(o *C.ZXing_WriterOptions) { C.ZXing_WriterOptions_setAddHRT(o, C.bool(v)) }
}

// WithQuietZones sets whether to add quiet zones around the barcode.
func WithQuietZones(v bool) WriterOption {
	return func(o *C.ZXing_WriterOptions) { C.ZXing_WriterOptions_setAddQuietZones(o, C.bool(v)) }
}

func createAndConfigureWriterOptions(opts ...WriterOption) *C.ZXing_WriterOptions {
	wo := C.ZXing_WriterOptions_new()
	if wo == nil {
		panic("failed to create WriterOptions") // only out of memory should cause this, so panic is reasonable
	}
	for _, opt := range opts {
		opt(wo)
	}
	return wo
}

// ToSVG generates an SVG string representation of the barcode with optional configuration options.
func (b *Barcode) ToSVG(opts ...WriterOption) (string, error) {
	wo := createAndConfigureWriterOptions(opts...)
	defer C.ZXing_WriterOptions_delete(wo)

	s := C.ZXing_WriteBarcodeToSVG(b.ptr, wo)
	if s == nil {
		return "", zxLastError()
	}
	return zxString2Go(s), nil
}

// ToImage generates a raster image representation of the barcode with optional configuration options.
func (b *Barcode) ToImage(opts ...WriterOption) (image.Image, error) {
	wo := createAndConfigureWriterOptions(opts...)
	defer C.ZXing_WriterOptions_delete(wo)

	return zxImage2Go(C.ZXing_WriteBarcodeToImage(b.ptr, wo))
}
