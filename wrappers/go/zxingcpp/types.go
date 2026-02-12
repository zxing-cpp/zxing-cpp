// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include "zxingcgo.h"
*/
import "C"

// ImageFormat describes how pixel bytes are interpreted.
type ImageFormat C.ZXing_ImageFormat

const (
	ImageFormatNone ImageFormat = C.ZXing_ImageFormat_None
	ImageFormatLum  ImageFormat = C.ZXing_ImageFormat_Lum
	ImageFormatLumA ImageFormat = C.ZXing_ImageFormat_LumA
	ImageFormatRGB  ImageFormat = C.ZXing_ImageFormat_RGB
	ImageFormatBGR  ImageFormat = C.ZXing_ImageFormat_BGR
	ImageFormatRGBA ImageFormat = C.ZXing_ImageFormat_RGBA
	ImageFormatARGB ImageFormat = C.ZXing_ImageFormat_ARGB
	ImageFormatBGRA ImageFormat = C.ZXing_ImageFormat_BGRA
	ImageFormatABGR ImageFormat = C.ZXing_ImageFormat_ABGR
)

// BarcodeFormat identifies a barcode format/symbology or a filter/group.
// Concrete values are generated in barcodeformat.go.
type BarcodeFormat C.ZXing_BarcodeFormat

// Binarizer specifies the algorithm used to convert a grayscale image to a binary bitmap for scanning.
type Binarizer C.ZXing_Binarizer

const (
	BinarizerLocalAverage    Binarizer = C.ZXing_Binarizer_LocalAverage
	BinarizerGlobalHistogram Binarizer = C.ZXing_Binarizer_GlobalHistogram
	BinarizerFixedThreshold  Binarizer = C.ZXing_Binarizer_FixedThreshold
	BinarizerBoolCast        Binarizer = C.ZXing_Binarizer_BoolCast
)

// EanAddOnSymbol specifies whether to ignore, read or require EAN-2/5 add-on symbols while scanning EAN/UPC codes.
type EanAddOnSymbol C.ZXing_EanAddOnSymbol

const (
	EanAddOnSymbolIgnore  EanAddOnSymbol = C.ZXing_EanAddOnSymbol_Ignore
	EanAddOnSymbolRead    EanAddOnSymbol = C.ZXing_EanAddOnSymbol_Read
	EanAddOnSymbolRequire EanAddOnSymbol = C.ZXing_EanAddOnSymbol_Require
)

// TextMode specifies how the decoded byte content of a barcode should be transcoded to text.
type TextMode C.ZXing_TextMode

const (
	TextModePlain   TextMode = C.ZXing_TextMode_Plain
	TextModeECI     TextMode = C.ZXing_TextMode_ECI
	TextModeHRI     TextMode = C.ZXing_TextMode_HRI
	TextModeEscaped TextMode = C.ZXing_TextMode_Escaped
	TextModeHex     TextMode = C.ZXing_TextMode_Hex
	TextModeHexECI  TextMode = C.ZXing_TextMode_HexECI
)

// ContentType provides a hint to the type of content encoded in a barcode, such as text, binary data, etc.
type ContentType C.ZXing_ContentType

const (
	ContentTypeText       ContentType = C.ZXing_ContentType_Text
	ContentTypeBinary     ContentType = C.ZXing_ContentType_Binary
	ContentTypeMixed      ContentType = C.ZXing_ContentType_Mixed
	ContentTypeGS1        ContentType = C.ZXing_ContentType_GS1
	ContentTypeISO15434   ContentType = C.ZXing_ContentType_ISO15434
	ContentTypeUnknownECI ContentType = C.ZXing_ContentType_UnknownECI
)

// ErrorType describes the type of error (if any) that occurred during reading of a barcode.
type ErrorType C.ZXing_ErrorType

const (
	ErrorTypeNone        ErrorType = C.ZXing_ErrorType_None
	ErrorTypeFormat      ErrorType = C.ZXing_ErrorType_Format
	ErrorTypeChecksum    ErrorType = C.ZXing_ErrorType_Checksum
	ErrorTypeUnsupported ErrorType = C.ZXing_ErrorType_Unsupported
)

// PointI is an integer 2D point used in Position.
type PointI struct {
	X C.int
	Y C.int
}

// Position represents a quadrilateral describing a barcode location in an image.
type Position struct {
	TopLeft     PointI
	TopRight    PointI
	BottomRight PointI
	BottomLeft  PointI
}

// BarcodeError represents an error encountered while reading a barcode (if any).
type BarcodeError struct {
	Type ErrorType
	Msg  string
}

func (e BarcodeError) String() string {
	return e.Msg
}

func (ct ContentType) String() string {
	return zxString2Go(C.ZXing_ContentTypeToString(C.ZXing_ContentType(ct)))
}

func (p Position) String() string {
	cp := C.ZXing_Position{
		topLeft:     C.ZXing_PointI{x: p.TopLeft.X, y: p.TopLeft.Y},
		topRight:    C.ZXing_PointI{x: p.TopRight.X, y: p.TopRight.Y},
		bottomRight: C.ZXing_PointI{x: p.BottomRight.X, y: p.BottomRight.Y},
		bottomLeft:  C.ZXing_PointI{x: p.BottomLeft.X, y: p.BottomLeft.Y},
	}
	return zxString2Go(C.ZXing_PositionToString(cp))
}
