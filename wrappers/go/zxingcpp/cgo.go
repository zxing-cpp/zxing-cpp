// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

// Package zxingcpp provides Go bindings for the zxing-cpp barcode scanning and generation library.
//
// The main type is Barcode, which represents a decoded or created barcode symbol, providing access
// to its content, format, position, and other metadata. To read barcodes from an image, use the ReadBarcodes()
// function, to create a new barcode, use the CreateBarcode() function.
package zxingcpp

// To check for proper docstring formatting, call go run honnef.co/go/tools/cmd/staticcheck@latest -checks=all ./...

/*
#cgo !zxing_local pkg-config: zxing

#cgo zxing_local CFLAGS: -I${SRCDIR}/../../../core/src -I${SRCDIR}/../../../build/core
#cgo zxing_local LDFLAGS: -L${SRCDIR}/../../../build/core -lZXing
#cgo zxing_local,darwin LDFLAGS: -Wl,-rpath,${SRCDIR}/../../../build/core
#cgo zxing_local,linux LDFLAGS: -Wl,-rpath,${SRCDIR}/../../../build/core

#include "zxingcgo.h"
*/
import "C"

// Version returns the zxing-cpp version string of the linked native library.
func Version() string {
	return C.GoString(C.ZXing_Version())
}
