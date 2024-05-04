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
	var readerInit: Boolean
		get() = ZXing_CreatorOptions_getReaderInit(cValue)
		set(value) = ZXing_CreatorOptions_setReaderInit(cValue, value)
	var forceSquareDataMatrix: Boolean
		get() = ZXing_CreatorOptions_getForceSquareDataMatrix(cValue)
		set(value) = ZXing_CreatorOptions_setForceSquareDataMatrix(cValue, value)
	var ecLevel: String
		get() = ZXing_CreatorOptions_getEcLevel(cValue)?.toKStringNullPtrHandledAndFree() ?: ""
		set(value) = ZXing_CreatorOptions_setEcLevel(cValue, value)

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
	var withHRT: Boolean
		get() = ZXing_WriterOptions_getWithHRT(cValue)
		set(value) = ZXing_WriterOptions_setWithHRT(cValue, value)
	var withQuietZones: Boolean
		get() = ZXing_WriterOptions_getWithQuietZones(cValue)
		set(value) = ZXing_WriterOptions_setWithQuietZones(cValue, value)

	val cValue: CValuesRef<ZXing_WriterOptions>? = ZXing_WriterOptions_new()

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cleaner = createCleaner(cValue) { ZXing_WriterOptions_delete(it) }
}
