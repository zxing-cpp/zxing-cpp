package zxingcpp

import cnames.structs.ZXing_ImageView
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
	val cValue: CPointer<ZXing_ImageView>? =
		ZXing_ImageView_new(
			pinnedData.addressOf(0),
			width,
			height,
			format.rawValue,
			rowStride,
			pixStride
		)

	override fun close() {
		ZXing_ImageView_delete(cValue)
		pinnedData.unpin()
	}
}

@OptIn(ExperimentalForeignApi::class)
enum class ImageFormat(internal val rawValue: ZXing_ImageFormat) {
	None(ZXing_ImageFormat_None),
	Lum(ZXing_ImageFormat_Lum),
	RGB(ZXing_ImageFormat_RGB),
	BGR(ZXing_ImageFormat_BGR),
	RGBX(ZXing_ImageFormat_RGBX),
	XRGB(ZXing_ImageFormat_XRGB),
	BGRX(ZXing_ImageFormat_BGRX),
	XBGR(ZXing_ImageFormat_XBGR)
}

@OptIn(ExperimentalForeignApi::class)
fun ZXing_ImageFormat.parseIntoImageFormat(): ImageFormat? =
	ImageFormat.entries.firstOrNull { it.rawValue == this }
