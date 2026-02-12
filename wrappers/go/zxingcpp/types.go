/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

// ImageFormat describes how pixel bytes are interpreted.
type ImageFormat uint32

const (
	ImageFormatNone ImageFormat = 0
	ImageFormatLum  ImageFormat = 0x01000000
	ImageFormatLumA ImageFormat = 0x02000000
	ImageFormatRGB  ImageFormat = 0x03000102
	ImageFormatBGR  ImageFormat = 0x03020100
	ImageFormatRGBA ImageFormat = 0x04000102
	ImageFormatARGB ImageFormat = 0x04010203
	ImageFormatBGRA ImageFormat = 0x04020100
	ImageFormatABGR ImageFormat = 0x04030201
)

// BarcodeFormat identifies a barcode format/symbology or a filter/group.
// Concrete values are generated in barcodeformat_gen.go.
type BarcodeFormat uint32

type Binarizer int

const (
	BinarizerLocalAverage Binarizer = iota
	BinarizerGlobalHistogram
	BinarizerFixedThreshold
	BinarizerBoolCast
)

type EanAddOnSymbol int

const (
	EanAddOnSymbolIgnore EanAddOnSymbol = iota
	EanAddOnSymbolRead
	EanAddOnSymbolRequire
)

type TextMode int

const (
	TextModePlain TextMode = iota
	TextModeECI
	TextModeHRI
	TextModeEscaped
	TextModeHex
	TextModeHexECI
)

type ContentType int

const (
	ContentTypeText ContentType = iota
	ContentTypeBinary
	ContentTypeMixed
	ContentTypeGS1
	ContentTypeISO15434
	ContentTypeUnknownECI
)

type ErrorType int

const (
	ErrorTypeNone ErrorType = iota
	ErrorTypeFormat
	ErrorTypeChecksum
	ErrorTypeUnsupported
)

type PointI struct {
	X int
	Y int
}

type Position struct {
	TopLeft     PointI
	TopRight    PointI
	BottomRight PointI
	BottomLeft  PointI
}

type BarcodeError struct {
	Type ErrorType
	Msg  string
}

func (e BarcodeError) Error() string {
	return e.Msg
}
