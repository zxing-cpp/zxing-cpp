/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.cinterop.*

@OptIn(ExperimentalForeignApi::class)
enum class BarcodeFormat(internal val rawValue: UInt) {
	None(ZXing_BarcodeFormat_None),
	Aztec(ZXing_BarcodeFormat_Aztec),
	Codabar(ZXing_BarcodeFormat_Codabar),
	Code39(ZXing_BarcodeFormat_Code39),
	Code93(ZXing_BarcodeFormat_Code93),
	Code128(ZXing_BarcodeFormat_Code128),
	DataBar(ZXing_BarcodeFormat_DataBar),
	DataBarExpanded(ZXing_BarcodeFormat_DataBarExpanded),
	DataBarLimited(ZXing_BarcodeFormat_DataBarLimited),
	DataMatrix(ZXing_BarcodeFormat_DataMatrix),
	DXFilmEdge(ZXing_BarcodeFormat_DXFilmEdge),
	EAN8(ZXing_BarcodeFormat_EAN8),
	EAN13(ZXing_BarcodeFormat_EAN13),
	ITF(ZXing_BarcodeFormat_ITF),
	MaxiCode(ZXing_BarcodeFormat_MaxiCode),
	PDF417(ZXing_BarcodeFormat_PDF417),
	QRCode(ZXing_BarcodeFormat_QRCode),
	MicroQrCode(ZXing_BarcodeFormat_MicroQRCode),
	RMQRCode(ZXing_BarcodeFormat_RMQRCode),
	UPCA(ZXing_BarcodeFormat_UPCA),
	UPCE(ZXing_BarcodeFormat_UPCE),

	LinearCodes(ZXing_BarcodeFormat_LinearCodes),
	MatrixCodes(ZXing_BarcodeFormat_MatrixCodes),
	Any(ZXing_BarcodeFormat_Any),

	Invalid(ZXing_BarcodeFormat_Invalid),
}

@OptIn(ExperimentalForeignApi::class)
fun ZXing_BarcodeFormat.parseIntoBarcodeFormat(): Set<BarcodeFormat> =
	BarcodeFormat.entries.filter { this.or(it.rawValue) == this }.toSet()

@OptIn(ExperimentalForeignApi::class)
fun Iterable<BarcodeFormat>.toValue(): ZXing_BarcodeFormat =
	this.map { it.rawValue }.reduce { acc, format -> acc.or(format) }
