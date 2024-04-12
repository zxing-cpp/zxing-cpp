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

@OptIn(ExperimentalForeignApi::class, ExperimentalNativeApi::class)
class ImageView(
	val cValue: CValuesRef<ZXing_ImageView>, private val pinnedData: Pinned<ByteArray>? = null,
	@Suppress("unused")
	private val cValueCleaner: Cleaner = createCleaner(cValue) { ZXing_ImageView_delete(it) },
	@Suppress("unused")
	private val pinnedDataCleaner: Cleaner? = pinnedData?.let { createCleaner(pinnedData) { it.unpin() } }
) {

	constructor(
		pinnedData: Pinned<ByteArray>,
		pinnedDataCleaner: Cleaner? = createCleaner(pinnedData) { it.unpin() },
		width: Int,
		height: Int,
		format: ImageFormat,
		rowStride: Int = 0,
		pixStride: Int = 0,
	) : this(
		cValue = ZXing_ImageView_new_checked(
			pinnedData.addressOf(0).reinterpret(),
			pinnedData.get().size,
			width,
			height,
			format.cValue,
			rowStride,
			pixStride,
		) ?: error("Failed to create ZXing_ImageView"),
		pinnedData = pinnedData,
		pinnedDataCleaner = pinnedDataCleaner
	)

	constructor(
		data: ByteArray,
		width: Int,
		height: Int,
		format: ImageFormat,
		rowStride: Int = 0,
		pixStride: Int = 0,
	) : this(
		pinnedData = data.pin(),
		width = width,
		height = height,
		format = format,
		rowStride = rowStride,
		pixStride = pixStride,
	)
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
class Image(val cValue: CValuesRef<ZXing_Image>) {
	val data: ByteArray
		get() = ZXing_Image_data(cValue)?.run {
			readBytes(width * height).also { ZXing_free(this) }
		}?.takeUnless { it.isEmpty() } ?: throw OutOfMemoryError()
	val width: Int get() = ZXing_Image_width(cValue)
	val height: Int get() = ZXing_Image_height(cValue)
	val format: ImageFormat
		get() = ZXing_Image_format(cValue).parseIntoImageFormat() ?: error(
			"Unknown format ${
				ZXing_Image_format(
					cValue
				)
			} for image"
		)

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	val cValueCleaner = createCleaner(cValue) { ZXing_Image_delete(it) }

	@Suppress("unchecked_cast")
	@OptIn(ExperimentalNativeApi::class)
	fun asImageView(): ImageView = ImageView(cValue as CValuesRef<ZXing_ImageView>, null, cValueCleaner, null)
}

@ExperimentalWriterApi
@OptIn(ExperimentalForeignApi::class)
fun CValuesRef<ZXing_Image>.toKObject(): Image = Image(this)
