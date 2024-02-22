/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import cnames.structs.ZXing_ImageView
import kotlinx.cinterop.*
import zxingcpp.cinterop.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.native.ref.createCleaner

@OptIn(ExperimentalForeignApi::class)
class ImageView(
	val data: ByteArray,
	val width: Int,
	val height: Int,
	val format: ImageFormat,
	val rowStride: Int = 0,
	val pixStride: Int = 0,
) {
	private val pinnedData = data.pin()
	val cValue: CPointer<ZXing_ImageView>? =
		ZXing_ImageView_new_checked(
			pinnedData.addressOf(0).reinterpret(),
			data.size,
			width,
			height,
			format.cValue,
			rowStride,
			pixStride
		)

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cValueCleaner = createCleaner(cValue) { ZXing_ImageView_delete(it) }

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val pinnedDataCleaner = createCleaner(pinnedData) { it.unpin() }
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
