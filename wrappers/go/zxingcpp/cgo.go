/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

/*
#cgo CFLAGS: -I${SRCDIR}/include -I${SRCDIR}/../../../core/src

// Link against the native ZXing library built by CMake.
// Users typically provide a -L via CGO_LDFLAGS or env vars.
#cgo darwin LDFLAGS: -lZXing -lc++
#cgo linux  LDFLAGS: -lZXing -lstdc++

#include <stdlib.h>
#include "ZXingC.h"
*/
import "C"

import (
	"errors"
	"runtime"
	"unsafe"
)

func lastErrorString() (string, bool) {
	ptr := C.ZXing_LastErrorMsg()
	if ptr == nil {
		return "", false
	}
	s := C.GoString(ptr)
	C.ZXing_free(unsafe.Pointer(ptr))
	return s, true
}

func errorFromLast(defaultMsg string) error {
	if msg, ok := lastErrorString(); ok && msg != "" {
		return errors.New(msg)
	}
	if defaultMsg == "" {
		defaultMsg = "ZXing C-API call failed"
	}
	return errors.New(defaultMsg)
}

func freeZXing(ptr unsafe.Pointer) {
	if ptr != nil {
		C.ZXing_free(ptr)
	}
}

func setFinalizer(obj any, fn any) {
	// keep in one place for easy grepping; Go finalizers are best-effort.
	runtime.SetFinalizer(obj, fn)
}
