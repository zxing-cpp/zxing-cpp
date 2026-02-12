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
	"image/draw"
	"runtime"
	"unsafe"
)

// ImageView is a non-owning view into pixel data used for barcode detection.
//
// To stay safe with cgo pointer rules, NewImageView copies the buffer into C memory.
// Call Close when you are done.
type ImageView struct {
	ptr     *C.ZXing_ImageView
	cbuf    unsafe.Pointer
	cbufLen int
	ownsBuf bool
}

func NewImageView(data []byte, width, height int, format ImageFormat, rowStride, pixStride int) (*ImageView, error) {
	if len(data) == 0 {
		return nil, fmt.Errorf("image data is empty")
	}
	cbuf := C.CBytes(data)
	if cbuf == nil {
		return nil, fmt.Errorf("failed to allocate C memory for image")
	}

	iv := C.ZXing_ImageView_new_checked(
		(*C.uint8_t)(cbuf),
		C.int(len(data)),
		C.int(width),
		C.int(height),
		C.ZXing_ImageFormat(format),
		C.int(rowStride),
		C.int(pixStride),
	)
	if iv == nil {
		C.free(cbuf)
		return nil, errorFromLast("failed to create ImageView")
	}

	v := &ImageView{ptr: iv, cbuf: cbuf, cbufLen: len(data), ownsBuf: true}
	setFinalizer(v, (*ImageView).finalize)
	return v, nil
}

// NewImageViewFromC wraps a pointer to C-allocated memory.
//
// The provided memory must remain valid for as long as the ImageView is used.
// The wrapper will not free the buffer.
func NewImageViewFromC(data unsafe.Pointer, dataLen int, width, height int, format ImageFormat, rowStride, pixStride int) (*ImageView, error) {
	if data == nil || dataLen <= 0 {
		return nil, fmt.Errorf("data pointer/length is invalid")
	}
	iv := C.ZXing_ImageView_new_checked(
		(*C.uint8_t)(data),
		C.int(dataLen),
		C.int(width),
		C.int(height),
		C.ZXing_ImageFormat(format),
		C.int(rowStride),
		C.int(pixStride),
	)
	if iv == nil {
		return nil, errorFromLast("failed to create ImageView")
	}
	v := &ImageView{ptr: iv, cbuf: data, cbufLen: dataLen, ownsBuf: false}
	setFinalizer(v, (*ImageView).finalize)
	return v, nil
}

// NewImageViewFromImage constructs an ImageView from a standard library image.Image.
//
// The function maps common concrete image layouts directly to matching ZXing image formats.
// For unsupported layouts it falls back to a grayscale conversion.
func NewImageViewFromImage(src image.Image) (*ImageView, error) {
	b := src.Bounds()
	width, height := b.Dx(), b.Dy()

	switch img := src.(type) {
	case *image.Gray:
		off := img.PixOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Pix[off:], width, height, ImageFormatLum, img.Stride, 1)

	case *image.RGBA:
		off := img.PixOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Pix[off:], width, height, ImageFormatRGBA, img.Stride, 4)

	case *image.NRGBA:
		off := img.PixOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Pix[off:], width, height, ImageFormatRGBA, img.Stride, 4)

	case *image.RGBA64:
		off := img.PixOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Pix[off:], width, height, ImageFormatRGBA, img.Stride, 8)

	case *image.NRGBA64:
		off := img.PixOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Pix[off:], width, height, ImageFormatRGBA, img.Stride, 8)

	case *image.YCbCr:
		off := img.YOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Y[off:], width, height, ImageFormatLum, img.YStride, 1)
	}

	gray := image.NewGray(b)
	draw.Draw(gray, b, src, b.Min, draw.Src)
	off := gray.PixOffset(b.Min.X, b.Min.Y)
	return NewImageView(gray.Pix[off:], width, height, ImageFormatLum, gray.Stride, 1)
}

func (iv *ImageView) finalize() {
	_ = iv.Close()
}

func (iv *ImageView) Close() error {
	if iv == nil {
		return nil
	}
	if iv.ptr != nil {
		C.ZXing_ImageView_delete(iv.ptr)
		iv.ptr = nil
	}
	if iv.ownsBuf && iv.cbuf != nil {
		C.free(iv.cbuf)
		iv.cbuf = nil
		iv.cbufLen = 0
		iv.ownsBuf = false
	}
	runtime.KeepAlive(iv)
	return nil
}

func (iv *ImageView) Crop(left, top, width, height int) {
	if iv == nil || iv.ptr == nil {
		return
	}
	C.ZXing_ImageView_crop(iv.ptr, C.int(left), C.int(top), C.int(width), C.int(height))
}

func (iv *ImageView) Rotate(degree int) {
	if iv == nil || iv.ptr == nil {
		return
	}
	C.ZXing_ImageView_rotate(iv.ptr, C.int(degree))
}

func toGoImageAndFree(cimg *C.ZXing_Image) (image.Image, error) {
	if cimg == nil {
		return nil, errorFromLast("failed to write barcode to image")
	}
	defer C.ZXing_Image_delete(cimg)

	width := int(C.ZXing_Image_width(cimg))
	height := int(C.ZXing_Image_height(cimg))
	format := ImageFormat(C.ZXing_Image_format(cimg))
	ptr := C.ZXing_Image_data(cimg)
	if ptr == nil {
		return nil, fmt.Errorf("ZXing image data is nil")
	}

	if format != ImageFormatLum {
		return nil, fmt.Errorf("unsupported writer image format %v", format)
	}

	// Writer output is grayscale (Lum) with tightly packed rows.
	pix := C.GoBytes(unsafe.Pointer(ptr), C.int(width*height))
	return &image.Gray{
		Pix:    pix,
		Stride: width,
		Rect:   image.Rect(0, 0, width, height),
	}, nil
}
