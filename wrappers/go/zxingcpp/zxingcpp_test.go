package zxingcpp

import (
	"strings"
	"testing"
)

func TestParseBarcodeFormat(t *testing.T) {
	fmtVal, err := ParseBarcodeFormat("QRCode")
	if err != nil {
		t.Fatalf("ParseBarcodeFormat(QRCode) failed: %v", err)
	}
	if fmtVal != BarcodeFormatQRCode {
		t.Fatalf("unexpected format: got %v, want %v", fmtVal, BarcodeFormatQRCode)
	}

	_, err = ParseBarcodeFormat("DefinitelyNotAFormat")
	if err == nil {
		t.Fatal("ParseBarcodeFormat should fail for invalid format")
	}
}

func TestBarcodeFormatsRoundTrip(t *testing.T) {
	formats, err := ParseBarcodeFormats("QRCode,DataMatrix")
	if err != nil {
		t.Fatalf("ParseBarcodeFormats failed: %v", err)
	}
	if len(formats) != 2 {
		t.Fatalf("unexpected format count: got %d, want 2", len(formats))
	}
	if formats[0] != BarcodeFormatQRCode || formats[1] != BarcodeFormatDataMatrix {
		t.Fatalf("unexpected parsed formats: %v", formats)
	}

	s := BarcodeFormatsString(formats)
	if !strings.Contains(s, "QR Code") || !strings.Contains(s, "Data Matrix") {
		t.Fatalf("BarcodeFormatsString(%v) = %q, expected both QR Code and Data Matrix", formats, s)
	}

	readable := ListBarcodeFormats(BarcodeFormatAllReadable)
	if len(readable) == 0 {
		t.Fatal("ListBarcodeFormats(AllReadable) returned empty list")
	}
}

func TestCreateAndReadBarcodeRoundTrip(t *testing.T) {
	bc, err := CreateBarcode("hello-zxing-go", BarcodeFormatQRCode, "EcLevel=L")
	if err != nil {
		t.Fatalf("CreateBarcode failed: %v", err)
	}
	defer bc.Close()

	if !bc.IsValid() {
		t.Fatal("created barcode is not valid")
	}
	if bc.Text() != "hello-zxing-go" {
		t.Fatalf("unexpected text: got %q", bc.Text())
	}
	if bc.Format() != BarcodeFormatQRCode {
		t.Fatalf("unexpected barcode format: got %v, want %v", bc.Format(), BarcodeFormatQRCode)
	}

	img, err := bc.ToImage(WithScale(4))
	if err != nil {
		t.Fatalf("ToImage failed: %v", err)
	}

	read, err := ReadBarcodes(img, WithFormats(BarcodeFormatQRCode), IsPure(true))
	if err != nil {
		t.Fatalf("ReadBarcodes failed: %v", err)
	}
	if len(read) == 0 {
		t.Fatal("ReadBarcodes returned no results")
	}
	defer func() {
		for _, r := range read {
			_ = r.Close()
		}
	}()

	if read[0].Text() != "hello-zxing-go" {
		t.Fatalf("unexpected decoded text: got %q", read[0].Text())
	}
	if read[0].Format() != BarcodeFormatQRCode {
		t.Fatalf("unexpected decoded format: got %v, want %v", read[0].Format(), BarcodeFormatQRCode)
	}
	if read[0].Extra("EcLevel") != "L" {
		t.Fatalf("unexpected EcLevel extra: got %q, want L", read[0].Extra("EcLevel"))
	}
}

func TestInvalidSourceTypeErrors(t *testing.T) {
	if _, err := CreateBarcode(123, BarcodeFormatQRCode); err == nil {
		t.Fatal("CreateBarcode should reject unsupported source type")
	}

	if _, err := ReadBarcodes(123); err == nil {
		t.Fatal("ReadBarcodes should reject unsupported source type")
	}
}
