/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import kotlinx.cinterop.*
import zxingcpp.cinterop.*

@OptIn(ExperimentalForeignApi::class)
enum class BarcodeFormat(internal val cValue: ZXing_BarcodeFormat) {
	None(ZXing_BarcodeFormat_None),
	Aztec(ZXing_BarcodeFormat_Aztec),
	Codabar(ZXing_BarcodeFormat_Codabar),
	Code39(ZXing_BarcodeFormat_Code39),
	Code93(ZXing_BarcodeFormat_Code93),
	Code128(ZXing_BarcodeFormat_Code128),
	DataBar(ZXing_BarcodeFormat_DataBar),
	DataBarOmD(ZXing_BarcodeFormat_DataBarOmD),
	DataBarLtd(ZXing_BarcodeFormat_DataBarLtd),
	DataBarExp(ZXing_BarcodeFormat_DataBarExp),
	DataMatrix(ZXing_BarcodeFormat_DataMatrix),
	DXFilmEdge(ZXing_BarcodeFormat_DXFilmEdge),
	EAN8(ZXing_BarcodeFormat_EAN8),
	EAN13(ZXing_BarcodeFormat_EAN13),
	ITF(ZXing_BarcodeFormat_ITF),
	MaxiCode(ZXing_BarcodeFormat_MaxiCode),
	PDF417(ZXing_BarcodeFormat_PDF417),
	QRCode(ZXing_BarcodeFormat_QRCode),
	MicroQRCode(ZXing_BarcodeFormat_MicroQRCode),
	RMQRCode(ZXing_BarcodeFormat_RMQRCode),
	UPCA(ZXing_BarcodeFormat_UPCA),
	UPCE(ZXing_BarcodeFormat_UPCE),

	All(ZXing_BarcodeFormat_All),
	AllReadable(ZXing_BarcodeFormat_AllReadable),
	AllCreatable(ZXing_BarcodeFormat_AllCreatable),
	AllLinear(ZXing_BarcodeFormat_AllLinear),
	AllStacked(ZXing_BarcodeFormat_AllStacked),
	AllMatrix(ZXing_BarcodeFormat_AllMatrix),
	AllGS1(ZXing_BarcodeFormat_AllGS1),

	Invalid(ZXing_BarcodeFormat_Invalid),
}

@OptIn(ExperimentalForeignApi::class)
fun ZXing_BarcodeFormat.toKObject(): BarcodeFormat {
	return BarcodeFormat.entries.first { it.cValue == this }
}

@OptIn(ExperimentalForeignApi::class)
fun CPointer<ZXing_BarcodeFormatVar>?.toKotlinSet(count: Int): Set<BarcodeFormat> {
	if (this == null || count <= 0) return emptySet()
	return (0 until count).map { i ->
		val rawVal = this[i]
		BarcodeFormat.entries.first { it.cValue == rawVal }
	}.toSet()
}

@OptIn(ExperimentalForeignApi::class)
fun Set<BarcodeFormat>.toCValues(): CValues<UIntVar> {
	val arr = this.map { it.cValue }.toUIntArray()
	return arr.toCValues()
}
