/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import cnames.structs.ZXing_ReaderOptions
import kotlinx.cinterop.*
import zxingcpp.cinterop.*
import zxingcpp.cinterop.ZXing_Binarizer.*
import zxingcpp.cinterop.ZXing_EanAddOnSymbol.*
import zxingcpp.cinterop.ZXing_TextMode.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.native.ref.createCleaner


@OptIn(ExperimentalForeignApi::class)
internal fun CPointer<ByteVar>?.toKStringNullPtrHandledAndFree(): String? = (this ?: throw OutOfMemoryError()).run {
	toKString().also { ZXing_free(this) }.ifEmpty { null }
}


@OptIn(ExperimentalForeignApi::class)
class BarcodeReader : ReaderOptions() {
	@Throws(BarcodeReadingException::class)
	fun read(imageView: ImageView): List<Barcode> = Companion.read(imageView, this)

	companion object {
		@Throws(BarcodeReadingException::class)
		fun read(imageView: ImageView, opts: ReaderOptions? = null): List<Barcode> =
			ZXing_ReadBarcodes(imageView.cValue, opts?.cValue)?.let { cValues -> cValues.toKObject().also { ZXing_Barcodes_delete(cValues) } }
				?: throw BarcodeReadingException(ZXing_LastErrorMsg()?.toKStringNullPtrHandledAndFree())
	}
}

class BarcodeReadingException(message: String?) : Exception("Failed to read barcodes: $message")


@OptIn(ExperimentalForeignApi::class)
open class ReaderOptions {
	var tryHarder: Boolean
		get() = ZXing_ReaderOptions_getTryHarder(cValue)
		set(value) = ZXing_ReaderOptions_setTryHarder(cValue, value)
	var tryRotate: Boolean
		get() = ZXing_ReaderOptions_getTryRotate(cValue)
		set(value) = ZXing_ReaderOptions_setTryRotate(cValue, value)
	var tryDownscale: Boolean
		get() = ZXing_ReaderOptions_getTryDownscale(cValue)
		set(value) = ZXing_ReaderOptions_setTryDownscale(cValue, value)
	var tryInvert: Boolean
		get() = ZXing_ReaderOptions_getTryInvert(cValue)
		set(value) = ZXing_ReaderOptions_setTryInvert(cValue, value)
	var isPure: Boolean
		get() = ZXing_ReaderOptions_getIsPure(cValue)
		set(value) = ZXing_ReaderOptions_setIsPure(cValue, value)
	var returnErrors: Boolean
		get() = ZXing_ReaderOptions_getReturnErrors(cValue)
		set(value) = ZXing_ReaderOptions_setReturnErrors(cValue, value)
	var binarizer: Binarizer
		get() = Binarizer.fromCValue(ZXing_ReaderOptions_getBinarizer(cValue))
		set(value) = ZXing_ReaderOptions_setBinarizer(cValue, value.cValue)
	var formats: Set<BarcodeFormat>
		get() = ZXing_ReaderOptions_getFormats(cValue).parseIntoBarcodeFormat()
		set(value) = ZXing_ReaderOptions_setFormats(cValue, value.toValue())
	var eanAddOnSymbol: EanAddOnSymbol
		get() = EanAddOnSymbol.fromCValue(ZXing_ReaderOptions_getEanAddOnSymbol(cValue))
		set(value) = ZXing_ReaderOptions_setEanAddOnSymbol(cValue, value.cValue)
	var textMode: TextMode
		get() = TextMode.fromCValue(ZXing_ReaderOptions_getTextMode(cValue))
		set(value) = ZXing_ReaderOptions_setTextMode(cValue, value.cValue)
	var minLineCount: Int
		get() = ZXing_ReaderOptions_getMinLineCount(cValue)
		set(value) = ZXing_ReaderOptions_setMinLineCount(cValue, value)
	var maxNumberOfSymbols: Int
		get() = ZXing_ReaderOptions_getMaxNumberOfSymbols(cValue)
		set(value) = ZXing_ReaderOptions_setMaxNumberOfSymbols(cValue, value)

	val cValue: CValuesRef<ZXing_ReaderOptions>? = ZXing_ReaderOptions_new()

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cleaner = createCleaner(cValue) { ZXing_ReaderOptions_delete(it) }
}

@OptIn(ExperimentalForeignApi::class)
enum class Binarizer(internal val cValue: ZXing_Binarizer) {
	LocalAverage(ZXing_Binarizer_LocalAverage),
	GlobalHistogram(ZXing_Binarizer_GlobalHistogram),
	FixedThreshold(ZXing_Binarizer_FixedThreshold),
	BoolCast(ZXing_Binarizer_BoolCast);

	companion object {
		fun fromCValue(cValue: ZXing_Binarizer): Binarizer {
			return entries.first { it.cValue == cValue }
		}
	}
}

@OptIn(ExperimentalForeignApi::class)
enum class EanAddOnSymbol(internal val cValue: ZXing_EanAddOnSymbol) {
	Ignore(ZXing_EanAddOnSymbol_Ignore),
	Read(ZXing_EanAddOnSymbol_Read),
	Require(ZXing_EanAddOnSymbol_Require);

	companion object {
		fun fromCValue(cValue: ZXing_EanAddOnSymbol): EanAddOnSymbol {
			return entries.first { it.cValue == cValue }
		}
	}
}

@OptIn(ExperimentalForeignApi::class)
enum class TextMode(internal val cValue: ZXing_TextMode) {
	Plain(ZXing_TextMode_Plain),
	ECI(ZXing_TextMode_ECI),
	HRI(ZXing_TextMode_HRI),
	Hex(ZXing_TextMode_Hex),
	Escaped(ZXing_TextMode_Escaped);

	companion object {
		fun fromCValue(cValue: ZXing_TextMode): TextMode {
			return entries.first { it.cValue == cValue }
		}
	}
}
