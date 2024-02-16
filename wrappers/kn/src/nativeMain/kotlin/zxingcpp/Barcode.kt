package zxingcpp

import cnames.structs.ZXing_Barcode
import cnames.structs.ZXing_Barcodes
import kotlinx.cinterop.*
import zxingcpp.cinterop.*
import zxingcpp.cinterop.ZXing_ContentType.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.native.ref.createCleaner

@OptIn(ExperimentalForeignApi::class)
enum class ContentType(internal val cValue: ZXing_ContentType) {
	Text(ZXing_ContentType_Text),
	Binary(ZXing_ContentType_Binary),
	Mixed(ZXing_ContentType_Mixed),
	GS1(ZXing_ContentType_GS1),
	ISO15434(ZXing_ContentType_ISO15434),
	UnknownECI(ZXing_ContentType_UnknownECI);
}

@OptIn(ExperimentalForeignApi::class)
fun ZXing_ContentType.toKObject(): ContentType {
	return ContentType.entries.first { it.cValue == this }
}

data class PointI(
	val x: Int,
	val y: Int
)

@OptIn(ExperimentalForeignApi::class)
fun ZXing_PointI.toKObject(): PointI = PointI(x, y)

data class Position(
	val topLeft: PointI,
	val topRight: PointI,
	val bottomRight: PointI,
	val bottomLeft: PointI,
)

@OptIn(ExperimentalForeignApi::class)
fun ZXing_Position.toKObject(): Position = Position(
	topLeft.toKObject(),
	topRight.toKObject(),
	bottomRight.toKObject(),
	bottomLeft.toKObject(),
)

@OptIn(ExperimentalForeignApi::class)
class Barcode(val cValue: CValuesRef<ZXing_Barcode>) {
	val isValid: Boolean
		get() = ZXing_Barcode_isValid(cValue)
	val errorMsg: String?
		get() = ZXing_Barcode_errorMsg(cValue)?.toKStringAndFree()
	val format: BarcodeFormat
		get() = ZXing_Barcode_format(cValue).parseIntoBarcodeFormat().first { it != BarcodeFormat.None }
	val contentType: ContentType
		get() = ZXing_Barcode_contentType(cValue).toKObject()

	fun bytes(len: Int): ByteArray? = ZXing_Barcode_bytes(cValue, cValuesOf(len))?.run {
		readBytes(len).also { ZXing_free(this) }
	}

	fun bytesECI(len: Int): ByteArray? = ZXing_Barcode_bytesECI(cValue, cValuesOf(len))?.run {
		readBytes(len).also { ZXing_free(this) }
	}

	val text: String?
		get() = ZXing_Barcode_text(cValue)?.toKStringAndFree()
	val ecLevel: String?
		get() = ZXing_Barcode_ecLevel(cValue)?.toKStringAndFree()
	val symbologyIdentifier: String?
		get() = ZXing_Barcode_symbologyIdentifier(cValue)?.toKStringAndFree()
	val position: Position
		get() = ZXing_Barcode_position(cValue).useContents { toKObject() }
	val orientation: Int
		get() = ZXing_Barcode_orientation(cValue)
	val hasECI: Boolean
		get() = ZXing_Barcode_hasECI(cValue)
	val isInverted: Boolean
		get() = ZXing_Barcode_isInverted(cValue)
	val isMirrored: Boolean
		get() = ZXing_Barcode_isMirrored(cValue)
	val lineCount: Int
		get() = ZXing_Barcode_lineCount(cValue)

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cleaner = createCleaner(cValue) { ZXing_Barcode_delete(it) }

	override fun toString(): String {
		return "Barcode(" +
			"isValid=$isValid, " +
			"errorMsg=$errorMsg, " +
			"format=$format, " +
			"contentType=$contentType, " +
			"text=$text, " +
			"ecLevel=$ecLevel, " +
			"symbologyIdentifier=$symbologyIdentifier, " +
			"position=$position, " +
			"orientation=$orientation, " +
			"hasECI=$hasECI, " +
			"isInverted=$isInverted, " +
			"isMirrored=$isMirrored, " +
			"lineCount=$lineCount" +
			")"
	}
}

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<ZXing_Barcode>.toKObject(): Barcode = Barcode(this)

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<ZXing_Barcodes>.toKObject(): List<Barcode> = mutableListOf<Barcode>().apply {
	for (i in 0..<ZXing_Barcodes_size(this@toKObject))
		ZXing_Barcodes_move(this@toKObject, i)?.toKObject()?.let { add(it) }
}.toList()
