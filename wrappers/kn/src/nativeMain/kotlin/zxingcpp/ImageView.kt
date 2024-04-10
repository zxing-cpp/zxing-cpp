/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import cnames.structs.ZXing_Image
import cnames.structs.ZXing_ImageView
import kotlinx.cinterop.*
import zxingcpp.cinterop.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.native.ref.Cleaner
import kotlin.native.ref.createCleaner

@OptIn(ExperimentalForeignApi::class)
abstract class ImageView {
	abstract val cValue: CValuesRef<ZXing_ImageView>

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	protected abstract val cValueCleaner: Cleaner
}

@OptIn(ExperimentalForeignApi::class)
open class ImageViewImplNoCopy(
	data: ByteArray,
	width: Int,
	height: Int,
	format: ImageFormat,
	rowStride: Int = 0,
	pixStride: Int = 0,
) : ImageView() {
	private val pinnedData = data.pin()

	final override val cValue: CPointer<ZXing_ImageView> = ZXing_ImageView_new_checked(
		pinnedData.addressOf(0).reinterpret(),
		data.size,
		width,
		height,
		format.cValue,
		rowStride,
		pixStride
	) ?: error("Failed to create ZXing_ImageView")

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val pinnedDataCleaner = createCleaner(pinnedData) { it.unpin() }

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	override val cValueCleaner = createCleaner(cValue) { ZXing_ImageView_delete(it) }
}


@OptIn(ExperimentalForeignApi::class)
enum class ImageFormat(internal val cValue: ZXing_ImageFormat) {
	None(ZXing_ImageFormat_None),
	Lum(ZXing_ImageFormat_Lum),
	LumA(ZXing_ImageFormat_LumA),
	RGB(ZXing_ImageFormat_RGB),
	BGR(ZXing_ImageFormat_BGR),
	RGBA(ZXing_ImageFormat_RGBA),
	ARGB(ZXing_ImageFormat_ARGB),
	BGRA(ZXing_ImageFormat_BGRA),
	ABGR(ZXing_ImageFormat_ABGR)
}

@OptIn(ExperimentalForeignApi::class)
fun ZXing_ImageFormat.parseIntoImageFormat(): ImageFormat? =
	ImageFormat.entries.firstOrNull { it.cValue == this }

@ExperimentalWriterApi
@OptIn(ExperimentalForeignApi::class)
class Image(val cValueImage: CValuesRef<ZXing_Image>) : ImageView() {
	@Suppress("unchecked_cast")
	override val cValue: CValuesRef<ZXing_ImageView> = cValueImage as CValuesRef<ZXing_ImageView>

	val data: ByteArray get() = ZXing_Image_data(cValueImage)?.run {
		readBytes(width * height).also { ZXing_free(this) }
	}?.takeUnless { it.isEmpty() } ?: throw OutOfMemoryError()
	val width: Int get() = ZXing_Image_width(cValueImage)
	val height: Int get() = ZXing_Image_height(cValueImage)
	val format: ImageFormat get() = ZXing_Image_format(cValueImage).parseIntoImageFormat() ?: error("Unknown format ${ZXing_Image_format(cValueImage)} for image")

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	override val cValueCleaner = createCleaner(cValueImage) { ZXing_Image_delete(it) }
}

@ExperimentalWriterApi
@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<ZXing_Image>.toKObject(): Image = Image(this)
