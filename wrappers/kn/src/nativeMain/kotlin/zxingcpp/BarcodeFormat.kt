/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import kotlinx.cinterop.*
import zxingcpp.cinterop.*

@OptIn(ExperimentalForeignApi::class)
enum class BarcodeFormat(internal val cValue: ZXing_BarcodeFormat) {
	None(ZXing_BarcodeFormat.ZXing_BarcodeFormat_None),
	Aztec(ZXing_BarcodeFormat.ZXing_BarcodeFormat_Aztec),
	Codabar(ZXing_BarcodeFormat.ZXing_BarcodeFormat_Codabar),
	Code39(ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code39),
	Code93(ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code93),
	Code128(ZXing_BarcodeFormat.ZXing_BarcodeFormat_Code128),
	DataBar(ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBar),
	DataBarOmni(ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarOmni),
	DataBarLtd(ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarLtd),
	DataBarExp(ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataBarExp),
	DataMatrix(ZXing_BarcodeFormat.ZXing_BarcodeFormat_DataMatrix),
	DXFilmEdge(ZXing_BarcodeFormat.ZXing_BarcodeFormat_DXFilmEdge),
	EAN8(ZXing_BarcodeFormat.ZXing_BarcodeFormat_EAN8),
	EAN13(ZXing_BarcodeFormat.ZXing_BarcodeFormat_EAN13),
	ITF(ZXing_BarcodeFormat.ZXing_BarcodeFormat_ITF),
	MaxiCode(ZXing_BarcodeFormat.ZXing_BarcodeFormat_MaxiCode),
	PDF417(ZXing_BarcodeFormat.ZXing_BarcodeFormat_PDF417),
	QRCode(ZXing_BarcodeFormat.ZXing_BarcodeFormat_QRCode),
	MicroQRCode(ZXing_BarcodeFormat.ZXing_BarcodeFormat_MicroQRCode),
	RMQRCode(ZXing_BarcodeFormat.ZXing_BarcodeFormat_RMQRCode),
	UPCA(ZXing_BarcodeFormat.ZXing_BarcodeFormat_UPCA),
	UPCE(ZXing_BarcodeFormat.ZXing_BarcodeFormat_UPCE),

	All(ZXing_BarcodeFormat.ZXing_BarcodeFormat_All),
	AllReadable(ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllReadable),
	AllCreatable(ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllCreatable),
	AllLinear(ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllLinear),
	AllStacked(ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllStacked),
	AllMatrix(ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllMatrix),
	AllGS1(ZXing_BarcodeFormat.ZXing_BarcodeFormat_AllGS1),

	Invalid(ZXing_BarcodeFormat.ZXing_BarcodeFormat_Invalid);

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
