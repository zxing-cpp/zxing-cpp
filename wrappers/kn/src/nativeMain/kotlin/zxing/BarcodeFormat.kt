package zxing

import kotlinx.cinterop.ExperimentalForeignApi
import zxing.cinterop.*

@OptIn(ExperimentalForeignApi::class)
enum class BarcodeFormat(internal val rawValue: UInt) {
	None(zxing_BarcodeFormat_None),
	Aztec(zxing_BarcodeFormat_Aztec),
	Codabar(zxing_BarcodeFormat_Codabar),
	Code39(zxing_BarcodeFormat_Code39),
	Code93(zxing_BarcodeFormat_Code93),
	Code128(zxing_BarcodeFormat_Code128),
	DataBar(zxing_BarcodeFormat_DataBar),
	DataBarExpanded(zxing_BarcodeFormat_DataBarExpanded),
	DataMatrix(zxing_BarcodeFormat_DataMatrix),
	DXFilmEdge(zxing_BarcodeFormat_DXFilmEdge),
	EAN8(zxing_BarcodeFormat_EAN8),
	EAN13(zxing_BarcodeFormat_EAN13),
	ITF(zxing_BarcodeFormat_ITF),
	MaxiCode(zxing_BarcodeFormat_MaxiCode),
	PDF417(zxing_BarcodeFormat_PDF417),
	QRCode(zxing_BarcodeFormat_QRCode),
	MicroQrCode(zxing_BarcodeFormat_MicroQRCode),
	RMQRCode(zxing_BarcodeFormat_RMQRCode),
	UPCA(zxing_BarcodeFormat_UPCA),
	UPCE(zxing_BarcodeFormat_UPCE),

	LinearCodes(zxing_BarcodeFormat_LinearCodes),
	MatrixCodes(zxing_BarcodeFormat_MatrixCodes),
	Any(zxing_BarcodeFormat_Any),

	Invalid(zxing_BarcodeFormat_Invalid),
}

@OptIn(ExperimentalForeignApi::class)
fun zxing_BarcodeFormat.parseIntoBarcodeFormat(): Set<BarcodeFormat> =
	BarcodeFormat.entries.filter { this.or(it.rawValue) == this }.toSet()

@OptIn(ExperimentalForeignApi::class)
fun Iterable<BarcodeFormat>.toValue(): zxing_BarcodeFormat =
	this.map { it.rawValue }.reduce { acc, format -> acc.or(format) }
