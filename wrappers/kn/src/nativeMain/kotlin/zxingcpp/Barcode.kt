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
	val errorMsg: String? by lazy {
		(ZXing_Barcode_errorMsg(cValue)?.toKStringAndFree()
			?: if (isValid) null else throw OutOfMemoryError())?.ifEmpty { null }
	}
	val format: BarcodeFormat by lazy {
		ZXing_Barcode_format(cValue).parseIntoBarcodeFormat().first { it != BarcodeFormat.None }
	}
	val contentType: ContentType by lazy {
		ZXing_Barcode_contentType(cValue).toKObject()
	}

	val bytes: ByteArray? by lazy {
		memScoped {
			val len = alloc<IntVar>()
			(ZXing_Barcode_bytes(cValue, len.ptr)?.run {
				readBytes(len.value).also { ZXing_free(this) }
			} ?: throw OutOfMemoryError()).takeUnless { it.isEmpty() }
		}
	}

	val bytesECI: ByteArray? by lazy {
		memScoped {
			val len = alloc<IntVar>()
			(ZXing_Barcode_bytesECI(cValue, len.ptr)?.run {
				readBytes(len.value).also { ZXing_free(this) }
			} ?: throw OutOfMemoryError()).takeUnless { it.isEmpty() }
		}
	}

	val text: String? by lazy {
		(ZXing_Barcode_text(cValue)?.toKStringAndFree() ?: throw OutOfMemoryError()).ifEmpty { null }
	}
	val ecLevel: String? by lazy {
		(ZXing_Barcode_ecLevel(cValue)?.toKStringAndFree() ?: throw OutOfMemoryError()).ifEmpty { null }
	}
	val symbologyIdentifier: String? by lazy {
		ZXing_Barcode_symbologyIdentifier(cValue)?.toKStringAndFree()
	}
	val position: Position by lazy {
		ZXing_Barcode_position(cValue).useContents { toKObject() }
	}
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
			"cValue=$cValue, " +
			"bytes=${bytes?.contentToString()}, " +
			"bytesECI=${bytesECI?.contentToString()}, " +
			"lineCount=$lineCount, " +
			"isMirrored=$isMirrored, " +
			"isInverted=$isInverted, " +
			"hasECI=$hasECI, " +
			"orientation=$orientation, " +
			"position=$position, " +
			"symbologyIdentifier=$symbologyIdentifier, " +
			"ecLevel=$ecLevel, " +
			"text=$text, " +
			"contentType=$contentType, " +
			"format=$format, " +
			"errorMsg=$errorMsg, " +
			"isValid=$isValid" +
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
