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

abstract class Barcode {
	abstract val isValid: Boolean
	abstract val errorMsg: String?
	abstract val format: BarcodeFormat
	abstract val contentType: ContentType
	abstract val text: String?
	abstract val ecLevel: String?
	abstract val symbologyIdentifier: String?
	abstract val position: Position
	abstract val orientation: Int
	abstract val hasECI: Boolean
	abstract val isInverted: Boolean
	abstract val isMirrored: Boolean
	abstract val lineCount: Int

	abstract fun bytes(len: Int): ByteArray?
	abstract fun bytesECI(len: Int): ByteArray?

	override fun equals(other: Any?): Boolean {
		if (this === other) return true
		if ((other == null) || other !is Barcode) return false

		if (isValid != other.isValid) return false
		if (errorMsg != other.errorMsg) return false
		if (format != other.format) return false
		if (contentType != other.contentType) return false
		if (text != other.text) return false
		if (ecLevel != other.ecLevel) return false
		if (symbologyIdentifier != other.symbologyIdentifier) return false
		if (position != other.position) return false
		if (orientation != other.orientation) return false
		if (hasECI != other.hasECI) return false
		if (isInverted != other.isInverted) return false
		if (isMirrored != other.isMirrored) return false
		if (lineCount != other.lineCount) return false

		return true
	}

	override fun hashCode(): Int {
		var result = isValid.hashCode()
		result = 31 * result + (errorMsg?.hashCode() ?: 0)
		result = 31 * result + format.hashCode()
		result = 31 * result + contentType.hashCode()
		result = 31 * result + (text?.hashCode() ?: 0)
		result = 31 * result + (ecLevel?.hashCode() ?: 0)
		result = 31 * result + (symbologyIdentifier?.hashCode() ?: 0)
		result = 31 * result + position.hashCode()
		result = 31 * result + orientation.hashCode()
		result = 31 * result + hasECI.hashCode()
		result = 31 * result + isInverted.hashCode()
		result = 31 * result + isMirrored.hashCode()
		result = 31 * result + lineCount.hashCode()
		return result
	}

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

class BarcodeImpl(
	override val isValid: Boolean,
	override val errorMsg: String?,
	override val format: BarcodeFormat,
	override val contentType: ContentType,
	override val text: String?,
	override val ecLevel: String?,
	override val symbologyIdentifier: String?,
	override val position: Position,
	override val orientation: Int,
	override val hasECI: Boolean,
	override val isInverted: Boolean,
	override val isMirrored: Boolean,
	override val lineCount: Int,
	private val bytes: ByteArray? = null,
	private val bytesECI: ByteArray? = null,
) : Barcode() {
	override fun bytes(len: Int): ByteArray? = bytes?.sliceArray(0 until len)
	override fun bytesECI(len: Int): ByteArray? = bytesECI?.sliceArray(0 until len)
}

@OptIn(ExperimentalForeignApi::class)
class BoundBarcode(val cValue: CValuesRef<zxing_Barcode>) : Barcode() {
	override val isValid: Boolean
		get() = zxing_Barcode_isValid(cValue)
	override val errorMsg: String?
		get() = zxing_Barcode_errorMsg(cValue)?.toKString()
	override val format: BarcodeFormat
		get() = zxing_Barcode_format(cValue).parseIntoBarcodeFormat().first { it != BarcodeFormat.None }
	override val contentType: ContentType
		get() = zxing_Barcode_contentType(cValue).toKObject()

	override fun bytes(len: Int): ByteArray? = zxing_Barcode_bytes(cValue, cValuesOf(len))?.readBytes(len)
	override fun bytesECI(len: Int): ByteArray? = zxing_Barcode_bytesECI(cValue, cValuesOf(len))?.readBytes(len)
	override val text: String?
		get() = zxing_Barcode_text(cValue)?.toKString()
	override val ecLevel: String?
		get() = zxing_Barcode_ecLevel(cValue)?.toKString()
	override val symbologyIdentifier: String?
		get() = zxing_Barcode_symbologyIdentifier(cValue)?.toKString()
	override val position: Position
		get() = zxing_Barcode_position(cValue).useContents { toKObject() }
	override val orientation: Int
		get() = zxing_Barcode_orientation(cValue)
	override val hasECI: Boolean
		get() = zxing_Barcode_hasECI(cValue)
	override val isInverted: Boolean
		get() = zxing_Barcode_isInverted(cValue)
	override val isMirrored: Boolean
		get() = zxing_Barcode_isMirrored(cValue)
	override val lineCount: Int
		get() = zxing_Barcode_lineCount(cValue)

	protected fun finalize() {
		zxing_Barcode_delete(cValue)
	}

}

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<zxing_Barcode>.toKObject(): BoundBarcode = BoundBarcode(this)

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<zxing_Barcodes>.toKObject(): List<BoundBarcode> = mutableListOf<BoundBarcode>().apply {
	for (i in 0..<zxing_Barcodes_size(this@toKObject))
		zxing_Barcodes_at(this@toKObject, i)?.toKObject()?.let { add(it) }
}.toList()
