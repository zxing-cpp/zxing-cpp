// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#include <stdlib.h>
#include "zxingcgo.h"
*/
import "C"

import (
	"unsafe"
)

// Barcode represents a decoded or created barcode symbol and provides access to
// its content, format, position, and metadata. Use ReadBarcodes or CreateBarcode
// to obtain Barcode instances. Convert a Barcode to an image or SVG with ToImage
// or ToSVG.
type Barcode struct {
	ptr *C.ZXing_Barcode
}

func (b *Barcode) Close() error {
	if b == nil || b.ptr == nil {
		return nil
	}
	C.ZXing_Barcode_delete(b.ptr)
	b.ptr = nil
	return nil
}

// IsValid reports whether the barcode contains a successfully decoded or created symbol.
func (b *Barcode) IsValid() bool { return b != nil && bool(C.ZXing_Barcode_isValid(b.ptr)) }

// Format returns the BarcodeFormat of the barcode.
func (b *Barcode) Format() BarcodeFormat { return BarcodeFormat(C.ZXing_Barcode_format(b.ptr)) }

// Symbology returns the symbology of the barcode format (e.g. EAN/UPC for EAN13, EAN8, UPCA).
func (b *Barcode) Symbology() BarcodeFormat { return BarcodeFormat(C.ZXing_Barcode_symbology(b.ptr)) }

// ContentType returns a hint to the type of content found (Text/Binary/GS1/etc.).
func (b *Barcode) ContentType() ContentType { return ContentType(C.ZXing_Barcode_contentType(b.ptr)) }

// Text returns the barcode content rendered to UTF-8 according to the ReaderOptions' TextMode.
func (b *Barcode) Text() string { return zxString2Go(C.ZXing_Barcode_text(b.ptr)) }

// Bytes returns the raw content without character set conversions.
func (b *Barcode) Bytes() []byte {
	return zxBytes2Go(func(n *C.int) *C.uint8_t { return C.ZXing_Barcode_bytes(b.ptr, n) })
}

// BytesECI returns the raw content following the ECI protocol.
func (b *Barcode) BytesECI() []byte {
	return zxBytes2Go(func(n *C.int) *C.uint8_t { return C.ZXing_Barcode_bytesECI(b.ptr, n) })
}

// SymbologyIdentifier returns the symbology identifier "]cm" where "c" is the symbology code and "m" the modifier.
func (b *Barcode) SymbologyIdentifier() string {
	return zxString2Go(C.ZXing_Barcode_symbologyIdentifier(b.ptr))
}

// Position returns the barcode position in the image as a quadrilateral of points (topLeft, topRight, bottomRight, bottomLeft).
func (b *Barcode) Position() Position {
	p := C.ZXing_Barcode_position(b.ptr)
	return Position{
		TopLeft:     PointI{X: p.topLeft.x, Y: p.topLeft.y},
		TopRight:    PointI{X: p.topRight.x, Y: p.topRight.y},
		BottomRight: PointI{X: p.bottomRight.x, Y: p.bottomRight.y},
		BottomLeft:  PointI{X: p.bottomLeft.x, Y: p.bottomLeft.y},
	}
}

// Orientation returns the orientation of the barcode in degrees.
func (b *Barcode) Orientation() int { return int(C.ZXing_Barcode_orientation(b.ptr)) }

// HasECI reports whether an ECI tag was found.
func (b *Barcode) HasECI() bool { return bool(C.ZXing_Barcode_hasECI(b.ptr)) }

// IsInverted reports whether the symbol is inverted (reversed reflectance).
func (b *Barcode) IsInverted() bool { return bool(C.ZXing_Barcode_isInverted(b.ptr)) }

// IsMirrored reports whether the symbol is mirrored (currently supported for QRCode and DataMatrix).
func (b *Barcode) IsMirrored() bool { return bool(C.ZXing_Barcode_isMirrored(b.ptr)) }

// LineCount returns how many lines have been detected for linear symbologies.
func (b *Barcode) LineCount() int { return int(C.ZXing_Barcode_lineCount(b.ptr)) }

// SequenceIndex returns the 0-based index of this symbol in a structured append sequence.
func (b *Barcode) SequenceIndex() int { return int(C.ZXing_Barcode_sequenceIndex(b.ptr)) }

// SequenceSize returns the number of symbols in a structured append sequence.
//
// If this is not part of a structured append sequence, the returned value is -1.
// If it is a structured append symbol but the total number of symbols is unknown,
// the returned value is 0 (see PDF417 if optional "Segment Count" not given).
func (b *Barcode) SequenceSize() int { return int(C.ZXing_Barcode_sequenceSize(b.ptr)) }

// SequenceID returns the sequence identifier for structured append sequences.
//
// If the symbology does not support this feature, an empty string is returned.
// For QR Code, this is the parity integer converted to a string. For PDF417 and
// DataMatrix, this is the file id.
func (b *Barcode) SequenceID() string { return zxString2Go(C.ZXing_Barcode_sequenceId(b.ptr)) }

// Error returns the error associated with the barcode, if any.
func (b *Barcode) Error() BarcodeError {
	return BarcodeError{Type: ErrorType(C.ZXing_Barcode_errorType(b.ptr)), Msg: zxString2Go(C.ZXing_Barcode_errorMsg(b.ptr))}
}

// Extra returns supplementary metadata associated with this barcode.
//
// The returned string contains additional, symbology-specific information serialized
// as a JSON object. If a key is provided, the corresponding value is returned. Keys
// are case-insensitive. If no information exists for the key, an empty string is
// returned.
func (b *Barcode) Extra(key ...string) string {
	var ckey *C.char
	if len(key) > 0 {
		ckey = C.CString(key[0])
		defer C.free(unsafe.Pointer(ckey))
	}
	return zxString2Go(C.ZXing_Barcode_extra(b.ptr, ckey))
}
