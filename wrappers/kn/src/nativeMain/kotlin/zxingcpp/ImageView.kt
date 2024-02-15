package zxingcpp

import cnames.structs.zxing_ImageView
import kotlinx.cinterop.CPointer
import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.pin
import zxingcpp.cinterop.*

@OptIn(ExperimentalForeignApi::class, ExperimentalStdlibApi::class)
data class ImageView(
	val data: UByteArray,
	val width: Int,
	val height: Int,
	val format: ImageFormat,
	val rowStride: Int = 0,
	val pixStride: Int = 0,
) : AutoCloseable {
	private val pinnedData = data.pin()
	val cValue: CPointer<zxing_ImageView>? =
		zxing_ImageView_new(
			pinnedData.addressOf(0),
			width,
			height,
			format.rawValue,
			rowStride,
			pixStride
		)

	override fun close() {
		zxing_ImageView_delete(cValue)
		pinnedData.unpin()
	}
}

@OptIn(ExperimentalForeignApi::class)
enum class ImageFormat(internal val rawValue: zxing_ImageFormat) {
	None(zxing_ImageFormat_None),
	Lum(zxing_ImageFormat_Lum),
	RGB(zxing_ImageFormat_RGB),
	BGR(zxing_ImageFormat_BGR),
	RGBX(zxing_ImageFormat_RGBX),
	XRGB(zxing_ImageFormat_XRGB),
	BGRX(zxing_ImageFormat_BGRX),
	XBGR(zxing_ImageFormat_XBGR)
}

@OptIn(ExperimentalForeignApi::class)
fun zxing_ImageFormat.parseIntoImageFormat(): ImageFormat? =
	ImageFormat.entries.firstOrNull { it.rawValue == this }
