/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package main

import (
	"errors"
	"fmt"
	"image/png"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/zxing-cpp/zxing-cpp/wrappers/go/zxingcpp"
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

	format, err := zxingcpp.ParseBarcodeFormat(formatStr)
	if err != nil {
		msg := fmt.Sprintf("invalid format %q: %v", formatStr, err)
		if supported, listErr := supportedCreatableFormats(); listErr == nil && supported != "" {
			msg += fmt.Sprintf("\nSupported formats: %s", supported)
		}
		return errors.New(msg)
	}

	fmt.Fprintf(stdout, "Creating barcode of format %s for text %q\n", format, text)

	copts, err := zxingcpp.NewCreatorOptions(format)
	if err != nil {
		return err
	}
	defer copts.Close()

	bc, err := zxingcpp.CreateBarcodeFromText(text, copts)
	if err != nil {
		return err
	}
	defer bc.Close()

	if strings.HasSuffix(strings.ToLower(outFile), ".svg") {
		return writeSVG(outFile, bc)
	}
	return writePNG(outFile, bc)
}

func writeSVG(outFile string, bc *zxingcpp.Barcode) error {
	wo, err := zxingcpp.NewWriterOptions()
	if err != nil {
		return err
	}
	defer wo.Close()
	wo.SetAddHRT(true)

	svg, err := bc.ToSVG(wo)
	if err != nil {
		return err
	}
	return os.WriteFile(outFile, []byte(svg), 0o644)
}

func writePNG(outFile string, bc *zxingcpp.Barcode) error {
	wo, err := zxingcpp.NewWriterOptions()
	if err != nil {
		return err
	}
	defer wo.Close()
	wo.SetScale(4)

	img, err := bc.ToImage(wo)
	if err != nil {
		return err
	}

	f, err := os.Create(outFile)
	if err != nil {
		return err
	}
	defer f.Close()

	return png.Encode(f, img)
}

func supportedCreatableFormats() (string, error) {
	fmts, err := zxingcpp.ListBarcodeFormats(zxingcpp.BarcodeFormatAllCreatable)
	if err != nil {
		return "", err
	}
	return zxingcpp.BarcodeFormatsString(fmts)
}
