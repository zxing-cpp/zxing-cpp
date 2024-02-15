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

data class Barcode(
	val isValid: Boolean,
	val errorMsg: String?,
	val format: BarcodeFormat,
	val contentType: ContentType,
	val bytes: ByteArray?,
	val bytesECI: ByteArray?,
	val text: String?,
	val ecLevel: String?,
	val symbologyIdentifier: String?,
	val position: Position,
	val orientation: Int,
	val hasECI: Boolean,
	val isInverted: Boolean,
	val isMirrored: Boolean,
	val lineCount: Int,
) {
	override fun equals(other: Any?): Boolean {
		if (this === other) return true
		if ((other == null) || (other::class != Barcode::class)) return false

		other as Barcode

		if (isValid != other.isValid) return false
		if (errorMsg != other.errorMsg) return false
		if (format != other.format) return false
		if (contentType != other.contentType) return false
		if (bytes != null) {
			if (other.bytes == null) return false
			if (!bytes.contentEquals(other.bytes)) return false
		} else if (other.bytes != null) return false
		if (bytesECI != null) {
			if (other.bytesECI == null) return false
			if (!bytesECI.contentEquals(other.bytesECI)) return false
		} else if (other.bytesECI != null) return false
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
		result = 31 * result + (bytes?.contentHashCode() ?: 0)
		result = 31 * result + (bytesECI?.contentHashCode() ?: 0)
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
}

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<zxing_Barcode>.toKObject(): Barcode = Barcode(
	zxing_Barcode_isValid(this),
	zxing_Barcode_errorMsg(this)?.toKStringAndFree(),
	zxing_Barcode_format(this).parseIntoBarcodeFormat().first { it != BarcodeFormat.None },
	zxing_Barcode_contentType(this).toKObject(),
	memScoped {
		val len = alloc<IntVar>()
		zxing_Barcode_bytes(this@toKObject, len.ptr)?.run {
			readBytes(len.value).also { zxing_free(this) }
		}
	},
	memScoped {
		val len = alloc<IntVar>()
		zxing_Barcode_bytesECI(this@toKObject, len.ptr)?.run {
			readBytes(len.value).also { zxing_free(this) }
		}
	},
	zxing_Barcode_text(this)?.toKStringAndFree(),
	zxing_Barcode_ecLevel(this)?.toKStringAndFree(),
	zxing_Barcode_symbologyIdentifier(this)?.toKStringAndFree(),
	zxing_Barcode_position(this).useContents { toKObject() },
	zxing_Barcode_orientation(this),
	zxing_Barcode_hasECI(this),
	zxing_Barcode_isInverted(this),
	zxing_Barcode_isMirrored(this),
	zxing_Barcode_lineCount(this),
)

@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<zxing_Barcodes>.toKObject(): List<Barcode> = mutableListOf<Barcode>().apply {
	for (i in 0..<zxing_Barcodes_size(this@toKObject))
		zxing_Barcodes_at(this@toKObject, i)?.toKObject()?.let { add(it) }
}.toList()
