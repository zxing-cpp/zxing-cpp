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
	"unsafe"
)

type Barcode struct {
	ptr *C.ZXing_Barcode
}

func (b *Barcode) finalize() { _ = b.Close() }

func (b *Barcode) Close() error {
	if b == nil {
		return nil
	}
	if b.ptr != nil {
		C.ZXing_Barcode_delete(b.ptr)
		b.ptr = nil
	}
	return nil
}

func (b *Barcode) IsValid() bool { return b != nil && b.ptr != nil && bool(C.ZXing_Barcode_isValid(b.ptr)) }

func (b *Barcode) Format() BarcodeFormat { return BarcodeFormat(C.ZXing_Barcode_format(b.ptr)) }

func (b *Barcode) Symbology() BarcodeFormat { return BarcodeFormat(C.ZXing_Barcode_symbology(b.ptr)) }

func (b *Barcode) ContentType() ContentType { return ContentType(C.ZXing_Barcode_contentType(b.ptr)) }

func (b *Barcode) SymbologyIdentifier() string {
	ptr := C.ZXing_Barcode_symbologyIdentifier(b.ptr)
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}

func (b *Barcode) Text() string {
	ptr := C.ZXing_Barcode_text(b.ptr)
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}

func (b *Barcode) Bytes() []byte {
	var n C.int
	ptr := C.ZXing_Barcode_bytes(b.ptr, &n)
	if ptr == nil {
		return nil
	}
	defer freeZXing(unsafe.Pointer(ptr))
	if n <= 0 {
		return nil
	}
	return C.GoBytes(unsafe.Pointer(ptr), n)
}

func (b *Barcode) BytesECI() []byte {
	var n C.int
	ptr := C.ZXing_Barcode_bytesECI(b.ptr, &n)
	if ptr == nil {
		return nil
	}
	defer freeZXing(unsafe.Pointer(ptr))
	if n <= 0 {
		return nil
	}
	return C.GoBytes(unsafe.Pointer(ptr), n)
}

func (b *Barcode) Position() Position {
	p := C.ZXing_Barcode_position(b.ptr)
	return Position{
		TopLeft:     PointI{X: int(p.topLeft.x), Y: int(p.topLeft.y)},
		TopRight:    PointI{X: int(p.topRight.x), Y: int(p.topRight.y)},
		BottomRight: PointI{X: int(p.bottomRight.x), Y: int(p.bottomRight.y)},
		BottomLeft:  PointI{X: int(p.bottomLeft.x), Y: int(p.bottomLeft.y)},
	}
}

func (b *Barcode) Orientation() int { return int(C.ZXing_Barcode_orientation(b.ptr)) }
func (b *Barcode) HasECI() bool     { return bool(C.ZXing_Barcode_hasECI(b.ptr)) }
func (b *Barcode) IsInverted() bool { return bool(C.ZXing_Barcode_isInverted(b.ptr)) }
func (b *Barcode) IsMirrored() bool { return bool(C.ZXing_Barcode_isMirrored(b.ptr)) }
func (b *Barcode) LineCount() int   { return int(C.ZXing_Barcode_lineCount(b.ptr)) }

func (b *Barcode) SequenceIndex() int { return int(C.ZXing_Barcode_sequenceIndex(b.ptr)) }
func (b *Barcode) SequenceSize() int  { return int(C.ZXing_Barcode_sequenceSize(b.ptr)) }

func (b *Barcode) SequenceID() string {
	ptr := C.ZXing_Barcode_sequenceId(b.ptr)
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}

func (b *Barcode) Error() BarcodeError {
	t := ErrorType(C.ZXing_Barcode_errorType(b.ptr))
	ptr := C.ZXing_Barcode_errorMsg(b.ptr)
	if ptr == nil {
		return BarcodeError{Type: t, Msg: ""}
	}
	msg := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return BarcodeError{Type: t, Msg: msg}
}

// Extra returns additional format-specific metadata.
//
// - If called with no arguments, it returns the full JSON metadata object.
// - If called with one key, it returns the value for that key.
func (b *Barcode) Extra(key ...string) string {
	var ckey *C.char
	if len(key) > 0 {
		ckey = C.CString(key[0])
		defer C.free(unsafe.Pointer(ckey))
	}
	ptr := C.ZXing_Barcode_extra(b.ptr, ckey)
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}

func (b *Barcode) String() string {
	if b == nil {
		return "<nil>"
	}
	txt := b.Text()
	return fmt.Sprintf("%s: %s", b.Format().String(), txt)
}
