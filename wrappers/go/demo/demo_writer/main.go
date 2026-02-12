// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

package main

import (
	"fmt"
	"image/png"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"

	zx "github.com/zxing-cpp/zxing-cpp/wrappers/go/zxingcpp"
)

func main() {
	log.SetFlags(0)
	if err := run(os.Args[1:], os.Stdout); err != nil {
		log.Fatal(err)
	}
}

func run(args []string, stdout io.Writer) error {
	if len(args) != 3 {
		return fmt.Errorf("usage: %s TEXT FORMAT OUTFILE(.svg|.png)", filepath.Base(os.Args[0]))
	}

	text := args[0]
	formatStr := args[1]
	outFile := args[2]

	format, err := zx.ParseBarcodeFormat(formatStr)
	if err != nil {
		fmts := zx.BarcodeFormatsString(zx.ListBarcodeFormats(zx.BarcodeFormatAllCreatable))
		return fmt.Errorf("%v\nSupported formats: %s", err, fmts)
	}

	fmt.Fprintf(stdout, "Creating barcode of format %s for text %q\n", format, text)

	bc, err := zx.CreateBarcode(text, format, zx.WithEcLevel("H"))
	if err != nil {
		return err
	}
	defer bc.Close()

	if strings.HasSuffix(strings.ToLower(outFile), ".svg") {
		svg, err := bc.ToSVG(zx.WithHRT(true))
		if err != nil {
			return err
		}
		return os.WriteFile(outFile, []byte(svg), 0o644)
	} else if strings.HasSuffix(strings.ToLower(outFile), ".png") {
		img, err := bc.ToImage(zx.WithScale(4))
		if err != nil {
			return err
		}

		f, err := os.Create(outFile)
		if err != nil {
			return err
		}
		defer f.Close()

		return png.Encode(f, img)
	} else {
		return fmt.Errorf("unsupported output file format: %s", outFile)
	}
}
