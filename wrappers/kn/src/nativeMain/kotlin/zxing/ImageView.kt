package zxing

import cnames.structs.zxing_ImageView
import kotlinx.cinterop.CPointer
import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.toCValues
import zxing.cinterop.*

@OptIn(ExperimentalForeignApi::class)
abstract class ImageView {
	abstract val data: UByteArray
	abstract val left: Int
	abstract val top: Int
	abstract val width: Int
	abstract val height: Int
	abstract val format: ImageFormat
	abstract val rotation: Int
	open val rowStride: Int = 0
	open val pixStride: Int = 0

	internal fun toCImageView(): CPointer<zxing_ImageView> =
		zxing_ImageView_new(data.toCValues(), width, height, format.rawValue, rowStride, pixStride).also {
			zxing_ImageView_rotate(it, rotation)
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
fun zxing_ImageFormat.parseIntoImageFormat(): ImageFormat =
	ImageFormat.entries.firstOrNull { it.rawValue == this }
