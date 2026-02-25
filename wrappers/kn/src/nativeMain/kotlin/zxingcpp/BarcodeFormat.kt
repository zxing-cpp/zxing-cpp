/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import kotlinx.cinterop.*
import zxingcpp.cinterop.*

@OptIn(ExperimentalForeignApi::class)
enum class BarcodeFormat(internal val cValue: ZXing_BarcodeFormat) {
	None           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_None),
	All            (ZXing_BarcodeFormat.ZXing_BarcodeFormat_All),
	AllReadable    (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllReadable),
	AllCreatable   (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllCreatable),
	AllLinear      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllLinear),
	AllMatrix      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllMatrix),
	AllGS1         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllGS1),
	AllRetail      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllRetail),
	AllIndustrial  (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllIndustrial),
	Codabar        (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Codabar),
	Code39         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code39),
	Code39Std      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code39Std),
	Code39Ext      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code39Ext),
	Code32         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code32),
	PZN            (ZXing_BarcodeFormat.ZXing_BarcodeFormat_PZN),
	Code93         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code93),
	Code128        (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code128),
	ITF            (ZXing_BarcodeFormat.ZXing_BarcodeFormat_ITF),
	ITF14          (ZXing_BarcodeFormat.ZXing_BarcodeFormat_ITF14),
	DataBar        (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBar),
	DataBarOmni    (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarOmni),
	DataBarStk     (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarStk),
	DataBarStkOmni (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarStkOmni),
	DataBarLtd     (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarLtd),
	DataBarExp     (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarExp),
	DataBarExpStk  (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarExpStk),
	EANUPC         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_EANUPC),
	EAN13          (ZXing_BarcodeFormat.ZXing_BarcodeFormat_EAN13),
	EAN8           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_EAN8),
	EAN5           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_EAN5),
	EAN2           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_EAN2),
	ISBN           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_ISBN),
	UPCA           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_UPCA),
	UPCE           (ZXing_BarcodeFormat.ZXing_BarcodeFormat_UPCE),
	OtherBarcode   (ZXing_BarcodeFormat.ZXing_BarcodeFormat_OtherBarcode),
	DXFilmEdge     (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DXFilmEdge),
	PDF417         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_PDF417),
	CompactPDF417  (ZXing_BarcodeFormat.ZXing_BarcodeFormat_CompactPDF417),
	MicroPDF417    (ZXing_BarcodeFormat.ZXing_BarcodeFormat_MicroPDF417),
	Aztec          (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Aztec),
	AztecCode      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AztecCode),
	AztecRune      (ZXing_BarcodeFormat.ZXing_BarcodeFormat_AztecRune),
	QRCode         (ZXing_BarcodeFormat.ZXing_BarcodeFormat_QRCode),
	QRCodeModel1   (ZXing_BarcodeFormat.ZXing_BarcodeFormat_QRCodeModel1),
	QRCodeModel2   (ZXing_BarcodeFormat.ZXing_BarcodeFormat_QRCodeModel2),
	MicroQRCode    (ZXing_BarcodeFormat.ZXing_BarcodeFormat_MicroQRCode),
	RMQRCode       (ZXing_BarcodeFormat.ZXing_BarcodeFormat_RMQRCode),
	DataMatrix     (ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataMatrix),
	MaxiCode       (ZXing_BarcodeFormat.ZXing_BarcodeFormat_MaxiCode),

	Invalid        (ZXing_BarcodeFormat.ZXing_BarcodeFormat_Invalid);

	companion object {
		fun fromCValue(cValue: ZXing_BarcodeFormat): BarcodeFormat {
			return entries.first { it.cValue == cValue }
		}
	}
}

@OptIn(ExperimentalForeignApi::class)
fun CPointer<ZXing_BarcodeFormat.Var>?.toKotlinSet(count: Int): Set<BarcodeFormat> {
	if (this == null || count <= 0) return emptySet()
	return (0 until count).map { i ->
		val rawVal = this[i]
		BarcodeFormat.fromCValue(rawVal.value)
	}.toSet()
}
