/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package main

import (
	"fmt"
	"image"
	_ "image/gif"
	_ "image/jpeg"
	_ "image/png"
	"io"
	"log"
	"os"
	"path/filepath"

	"github.com/zxing-cpp/zxing-cpp/wrappers/go/zxingcpp"
)

func main() {
	log.SetFlags(0)
	if err := run(os.Args[1:], os.Stdout); err != nil {
		log.Fatal(err)
	}
}

func run(args []string, stdout io.Writer) error {
	if len(args) < 1 || len(args) > 3 {
		return fmt.Errorf("usage: %s IMAGE [FORMATS] [fast]", filepath.Base(os.Args[0]))
	}

	filename := args[0]
	formatsArg := ""
	if len(args) >= 2 {
		formatsArg = args[1]
	}
	fast := len(args) >= 3

	f, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer f.Close()

	img, _, err := image.Decode(f)
	if err != nil {
		return err
	}

	opts, err := zxingcpp.NewReaderOptions()
	if err != nil {
		return err
	}
	defer opts.Close()

	if formatsArg != "" {
		formats, err := zxingcpp.ParseBarcodeFormats(formatsArg)
		if err != nil {
			return err
		}
		opts.SetFormats(formats)
	}

	opts.SetTryHarder(!fast)
	opts.SetTryInvert(!fast)
	opts.SetTryRotate(!fast)
	opts.SetTryDownscale(!fast)
	opts.SetReturnErrors(true)

	barcodes, err := zxingcpp.ReadBarcodes(img, opts)
	if err != nil {
		return err
	}

	if len(barcodes) == 0 {
		fmt.Fprintln(stdout, "No barcode found.")
		return nil
	}

	for _, bc := range barcodes {
		fmt.Fprintf(stdout, "Text:       %s\n", bc.Text())
		fmt.Fprintf(stdout, "Bytes:      %v\n", bc.Bytes())
		fmt.Fprintf(stdout, "Format:     %s\n", bc.Format())
		fmt.Fprintf(stdout, "Content:    %s\n", bc.ContentType())
		fmt.Fprintf(stdout, "Identifier: %s\n", bc.SymbologyIdentifier())
		fmt.Fprintf(stdout, "Error:      %s\n", bc.Error())
		fmt.Fprintf(stdout, "Rotation:   %d\n", bc.Orientation())
		fmt.Fprintf(stdout, "Position:   %s\n", bc.Position())
		fmt.Fprintf(stdout, "Extra:      %s\n", bc.Extra())
		bc.Close()
	}

	return nil
}
