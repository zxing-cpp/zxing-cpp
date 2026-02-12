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

import "unsafe"

// Version returns the zxing-cpp version string of the linked native library.
func Version() string {
	return C.GoString(C.ZXing_Version())
}

func (ct ContentType) String() string {
	ptr := C.ZXing_ContentTypeToString(C.ZXing_ContentType(ct))
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}

func (p Position) String() string {
	cp := C.ZXing_Position{
		topLeft:     C.ZXing_PointI{x: C.int(p.TopLeft.X), y: C.int(p.TopLeft.Y)},
		topRight:    C.ZXing_PointI{x: C.int(p.TopRight.X), y: C.int(p.TopRight.Y)},
		bottomRight: C.ZXing_PointI{x: C.int(p.BottomRight.X), y: C.int(p.BottomRight.Y)},
		bottomLeft:  C.ZXing_PointI{x: C.int(p.BottomLeft.X), y: C.int(p.BottomLeft.Y)},
	}
	ptr := C.ZXing_PositionToString(cp)
	if ptr == nil {
		return ""
	}
	s := C.GoString(ptr)
	freeZXing(unsafe.Pointer(ptr))
	return s
}
