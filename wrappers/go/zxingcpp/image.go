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
	"image/draw"
	"unsafe"
)

// ImageView is a non-owning view into pixel data used for barcode detection.
//
// ImageView keeps a reference to the original Go data to prevent garbage collection.
// Call Close when you are done.
type ImageView struct {
	ptr    *C.ZXing_ImageView
	goData []byte // keeps Go slice alive
}

func NewImageView(data []byte, width, height int, format ImageFormat, rowStride, pixStride int) (*ImageView, error) {
	if len(data) == 0 {
		return nil, fmt.Errorf("image data is empty")
	}

	iv := C.ZXing_ImageView_new_checked(
		(*C.uint8_t)(unsafe.Pointer(unsafe.SliceData(data))),
		C.int(len(data)),
		C.int(width),
		C.int(height),
		C.ZXing_ImageFormat(format),
		C.int(rowStride),
		C.int(pixStride),
	)
	if iv == nil {
		return nil, zxLastError()
	}

	return &ImageView{ptr: iv, goData: data}, nil
}

// NewImageViewFromC wraps a pointer to C-allocated memory.
//
// The provided memory must remain valid for as long as the ImageView is used.
// The wrapper will not free the buffer.
func NewImageViewFromC(data unsafe.Pointer, dataLen int, width, height int, format ImageFormat, rowStride, pixStride int) (*ImageView, error) {
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
		return nil, zxLastError()
	}
	return &ImageView{ptr: iv}, nil
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

	case *image.YCbCr:
		off := img.YOffset(b.Min.X, b.Min.Y)
		return NewImageView(img.Y[off:], width, height, ImageFormatLum, img.YStride, 1)
	}

	gray := image.NewGray(b)
	draw.Draw(gray, b, src, b.Min, draw.Src)
	off := gray.PixOffset(b.Min.X, b.Min.Y)
	return NewImageView(gray.Pix[off:], width, height, ImageFormatLum, gray.Stride, 1)
}

func (iv *ImageView) Close() error {
	if iv == nil || iv.ptr == nil {
		return nil
	}
	C.ZXing_ImageView_delete(iv.ptr)
	iv.ptr = nil
	iv.goData = nil
	return nil
}

// Crop crops the ImageView in-place to the specified rectangle.
func (iv *ImageView) Crop(left, top, width, height int) {
	if iv == nil || iv.ptr == nil {
		return
	}
	C.ZXing_ImageView_crop(iv.ptr, C.int(left), C.int(top), C.int(width), C.int(height))
}

// Rotate rotates the ImageView in-place by the specified degrees. Supported values are 0, 90, 180 and 270.
func (iv *ImageView) Rotate(degree int) {
	if iv == nil || iv.ptr == nil {
		return
	}
	C.ZXing_ImageView_rotate(iv.ptr, C.int(degree))
}

func zxImage2Go(cimg *C.ZXing_Image) (image.Image, error) {
	if cimg == nil {
		return nil, zxLastError()
	}
	defer C.ZXing_Image_delete(cimg)

	width := int(C.ZXing_Image_width(cimg))
	height := int(C.ZXing_Image_height(cimg))
	format := ImageFormat(C.ZXing_Image_format(cimg))
	ptr := C.ZXing_Image_data(cimg)
	if ptr == nil {
		return nil, fmt.Errorf("ZXing_Image_data() returned NULL")
	}

	if format != ImageFormatLum {
		return nil, fmt.Errorf("unsupported writer image format %v", format)
	}

	// Writer output is grayscale (Lum) with tightly packed rows.
	return &image.Gray{
		Pix:    C.GoBytes(unsafe.Pointer(ptr), C.int(width*height)),
		Stride: width,
		Rect:   image.Rect(0, 0, width, height),
	}, nil
}
