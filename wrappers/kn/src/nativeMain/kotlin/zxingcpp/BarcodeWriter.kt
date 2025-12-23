/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import cnames.structs.ZXing_CreatorOptions
import cnames.structs.ZXing_WriterOptions
import kotlinx.cinterop.*
import zxingcpp.cinterop.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.native.ref.createCleaner

// TODO: Remove this annotation when the API is stable
@RequiresOptIn(level = RequiresOptIn.Level.ERROR, message = "The Writer API is experimental and may change in the future.")
@Retention(AnnotationRetention.BINARY)
annotation class ExperimentalWriterApi

class BarcodeWritingException(message: String?) : Exception("Failed to write barcode: $message")

@ExperimentalWriterApi
@OptIn(ExperimentalForeignApi::class)
open class CreatorOptions(format: BarcodeFormat) {
	var format: BarcodeFormat
		get() = ZXing_CreatorOptions_getFormat(cValue).parseIntoBarcodeFormat().first()
		set(value) = ZXing_CreatorOptions_setFormat(cValue, value.rawValue)
	var options: String
		get() = ZXing_CreatorOptions_getOptions(cValue)?.toKStringNullPtrHandledAndFree() ?: ""
		set(value) = ZXing_CreatorOptions_setOptions(cValue, value)

	val cValue: CValuesRef<ZXing_CreatorOptions>? = ZXing_CreatorOptions_new(format.rawValue)

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cleaner = createCleaner(cValue) { ZXing_CreatorOptions_delete(it) }
}

@ExperimentalWriterApi
@OptIn(ExperimentalForeignApi::class)
open class WriterOptions {
	var scale: Int
		get() = ZXing_WriterOptions_getScale(cValue)
		set(value) = ZXing_WriterOptions_setScale(cValue, value)
	var sizeHint: Int
		get() = ZXing_WriterOptions_getSizeHint(cValue)
		set(value) = ZXing_WriterOptions_setSizeHint(cValue, value)
	var rotate: Int
		get() = ZXing_WriterOptions_getRotate(cValue)
		set(value) = ZXing_WriterOptions_setRotate(cValue, value)
	var addHRT: Boolean
		get() = ZXing_WriterOptions_getAddHRT(cValue)
		set(value) = ZXing_WriterOptions_setAddHRT(cValue, value)
	var addQuietZones: Boolean
		get() = ZXing_WriterOptions_getAddQuietZones(cValue)
		set(value) = ZXing_WriterOptions_setAddQuietZones(cValue, value)

	val cValue: CValuesRef<ZXing_WriterOptions>? = ZXing_WriterOptions_new()

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cleaner = createCleaner(cValue) { ZXing_WriterOptions_delete(it) }
}
