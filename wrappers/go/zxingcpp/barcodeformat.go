/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

const (
	BarcodeFormatInvalid         BarcodeFormat = 0xFFFF
	BarcodeFormatNone            BarcodeFormat = 0x0000
	BarcodeFormatAll             BarcodeFormat = 0x2A2A
	BarcodeFormatAllReadable     BarcodeFormat = 0x722A
	BarcodeFormatAllCreatable    BarcodeFormat = 0x772A
	BarcodeFormatAllLinear       BarcodeFormat = 0x6C2A
	BarcodeFormatAllMatrix       BarcodeFormat = 0x6D2A
	BarcodeFormatAllGS1          BarcodeFormat = 0x472A
	BarcodeFormatAllRetail       BarcodeFormat = 0x522A
	BarcodeFormatAllIndustrial   BarcodeFormat = 0x492A
	BarcodeFormatCodabar         BarcodeFormat = 0x2046
	BarcodeFormatCode39          BarcodeFormat = 0x2041
	BarcodeFormatCode39Std       BarcodeFormat = 0x7341
	BarcodeFormatCode39Ext       BarcodeFormat = 0x6541
	BarcodeFormatCode32          BarcodeFormat = 0x3241
	BarcodeFormatPZN             BarcodeFormat = 0x7041
	BarcodeFormatCode93          BarcodeFormat = 0x2047
	BarcodeFormatCode128         BarcodeFormat = 0x2043
	BarcodeFormatITF             BarcodeFormat = 0x2049
	BarcodeFormatITF14           BarcodeFormat = 0x3449
	BarcodeFormatDataBar         BarcodeFormat = 0x2065
	BarcodeFormatDataBarOmni     BarcodeFormat = 0x6F65
	BarcodeFormatDataBarStk      BarcodeFormat = 0x7365
	BarcodeFormatDataBarStkOmni  BarcodeFormat = 0x4F65
	BarcodeFormatDataBarLtd      BarcodeFormat = 0x6C65
	BarcodeFormatDataBarExp      BarcodeFormat = 0x6565
	BarcodeFormatDataBarExpStk   BarcodeFormat = 0x4565
	BarcodeFormatEANUPC          BarcodeFormat = 0x2045
	BarcodeFormatEAN13           BarcodeFormat = 0x3145
	BarcodeFormatEAN8            BarcodeFormat = 0x3845
	BarcodeFormatEAN5            BarcodeFormat = 0x3545
	BarcodeFormatEAN2            BarcodeFormat = 0x3245
	BarcodeFormatISBN            BarcodeFormat = 0x6945
	BarcodeFormatUPCA            BarcodeFormat = 0x6145
	BarcodeFormatUPCE            BarcodeFormat = 0x6545
	BarcodeFormatOtherBarcode    BarcodeFormat = 0x2058
	BarcodeFormatDXFilmEdge      BarcodeFormat = 0x7858
	BarcodeFormatPDF417          BarcodeFormat = 0x204C
	BarcodeFormatCompactPDF417   BarcodeFormat = 0x634C
	BarcodeFormatMicroPDF417     BarcodeFormat = 0x6D4C
	BarcodeFormatAztec           BarcodeFormat = 0x207A
	BarcodeFormatAztecCode       BarcodeFormat = 0x637A
	BarcodeFormatAztecRune       BarcodeFormat = 0x727A
	BarcodeFormatQRCode          BarcodeFormat = 0x2051
	BarcodeFormatQRCodeModel1    BarcodeFormat = 0x3151
	BarcodeFormatQRCodeModel2    BarcodeFormat = 0x3251
	BarcodeFormatMicroQRCode     BarcodeFormat = 0x6D51
	BarcodeFormatRMQRCode        BarcodeFormat = 0x7251
	BarcodeFormatDataMatrix      BarcodeFormat = 0x2064
	BarcodeFormatMaxiCode        BarcodeFormat = 0x2055
)
