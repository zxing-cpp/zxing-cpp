package zxingcpp

import cnames.structs.zxing_Barcode
import cnames.structs.zxing_Barcodes
import kotlinx.cinterop.*
import zxingcpp.cinterop.*
import zxingcpp.cinterop.zxing_ContentType.*

@OptIn(ExperimentalForeignApi::class)
enum class ContentType(internal val cValue: zxing_ContentType) {
	Text(zxing_ContentType_Text),
	Binary(zxing_ContentType_Binary),
	Mixed(zxing_ContentType_Mixed),
	GS1(zxing_ContentType_GS1),
	ISO15434(zxing_ContentType_ISO15434),
	UnknownECI(zxing_ContentType_UnknownECI);
}

@OptIn(ExperimentalForeignApi::class)
fun zxing_ContentType.toKObject(): ContentType {
	return ContentType.entries.first { it.cValue == this }
}

data class PointI(
	val x: Int,
	val y: Int
)

@OptIn(ExperimentalForeignApi::class)
fun zxing_PointI.toKObject(): PointI = PointI(x, y)

data class Position(
	val topLeft: PointI,
	val topRight: PointI,
	val bottomRight: PointI,
	val bottomLeft: PointI,
)

@OptIn(ExperimentalForeignApi::class)
fun zxing_Position.toKObject(): Position = Position(
	topLeft.toKObject(),
	topRight.toKObject(),
	bottomRight.toKObject(),
	bottomLeft.toKObject(),
)

@OptIn(ExperimentalForeignApi::class)
class Result(val cValue: CValuesRef<zxing_Barcode>) {
	val isValid: Boolean
		get() = zxing_Barcode_isValid(cValue)
	val errorMsg: String?
		get() = zxing_Barcode_errorMsg(cValue)?.toKString()
	val format: BarcodeFormat
		get() = zxing_Barcode_format(cValue).parseIntoBarcodeFormat().first { it != BarcodeFormat.None }
	val contentType: ContentType
		get() = zxing_Barcode_contentType(cValue).toKObject()

	fun bytes(len: Int): ByteArray? = zxing_Barcode_bytes(cValue, cValuesOf(len))?.readBytes(len)
	fun bytesECI(len: Int): ByteArray? = zxing_Barcode_bytesECI(cValue, cValuesOf(len))?.readBytes(len)
	val text: String?
		get() = zxing_Barcode_text(cValue)?.toKString()
	val ecLevel: String?
		get() = zxing_Barcode_ecLevel(cValue)?.toKString()
	val symbologyIdentifier: String?
		get() = zxing_Barcode_symbologyIdentifier(cValue)?.toKString()
	val position: Position
		get() = zxing_Barcode_position(cValue).useContents { toKObject() }
	val orientation: Int
		get() = zxing_Barcode_orientation(cValue)
	val hasECI: Boolean
		get() = zxing_Barcode_hasECI(cValue)
	val isInverted: Boolean
		get() = zxing_Barcode_isInverted(cValue)
	val isMirrored: Boolean
		get() = zxing_Barcode_isMirrored(cValue)
	val lineCount: Int
		get() = zxing_Barcode_lineCount(cValue)

}

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<zxing_Barcode>.toKObject(): Result = Result(this)

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<zxing_Barcodes>.toKObject(): List<Result> = mutableListOf<Result>().apply {
	for (i in 0..<zxing_Barcodes_size(this@toKObject))
		zxing_Barcodes_at(this@toKObject, i)?.toKObject()?.let { add(it) }
}.toList()
