package zxingcpp

import cnames.structs.zxing_ReaderOptions
import kotlinx.cinterop.CValuesRef
import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.cinterop.*
import zxingcpp.cinterop.zxing_Binarizer.*
import zxingcpp.cinterop.zxing_EanAddOnSymbol.*
import zxingcpp.cinterop.zxing_TextMode.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.native.ref.createCleaner

@OptIn(ExperimentalForeignApi::class)
class ReaderOptions {
	var tryHarder: Boolean
		get() = zxing_ReaderOptions_getTryHarder(cValue)
		set(value) = zxing_ReaderOptions_setTryHarder(cValue, value)
	var tryRotate: Boolean
		get() = zxing_ReaderOptions_getTryRotate(cValue)
		set(value) = zxing_ReaderOptions_setTryRotate(cValue, value)
	var tryDownscale: Boolean
		get() = zxing_ReaderOptions_getTryDownscale(cValue)
		set(value) = zxing_ReaderOptions_setTryDownscale(cValue, value)
	var tryInvert: Boolean
		get() = zxing_ReaderOptions_getTryInvert(cValue)
		set(value) = zxing_ReaderOptions_setTryInvert(cValue, value)
	var isPure: Boolean
		get() = zxing_ReaderOptions_getIsPure(cValue)
		set(value) = zxing_ReaderOptions_setIsPure(cValue, value)
	var returnErrors: Boolean
		get() = zxing_ReaderOptions_getReturnErrors(cValue)
		set(value) = zxing_ReaderOptions_setReturnErrors(cValue, value)
	var binarizer: Binarizer
		get() = Binarizer.fromCValue(zxing_ReaderOptions_getBinarizer(cValue))
		set(value) = zxing_ReaderOptions_setBinarizer(cValue, value.cValue)
	var formats: Set<BarcodeFormat>
		get() = zxing_ReaderOptions_getFormats(cValue).parseIntoBarcodeFormat()
		set(value) = zxing_ReaderOptions_setFormats(cValue, value.toValue())
	var eanAddOnSymbol: EanAddOnSymbol
		get() = EanAddOnSymbol.fromCValue(zxing_ReaderOptions_getEanAddOnSymbol(cValue))
		set(value) = zxing_ReaderOptions_setEanAddOnSymbol(cValue, value.cValue)
	var textMode: TextMode
		get() = TextMode.fromCValue(zxing_ReaderOptions_getTextMode(cValue))
		set(value) = zxing_ReaderOptions_setTextMode(cValue, value.cValue)
	var minLineCount: Int
		get() = zxing_ReaderOptions_getMinLineCount(cValue)
		set(value) = zxing_ReaderOptions_setMinLineCount(cValue, value)
	var maxNumberOfSymbols: Int
		get() = zxing_ReaderOptions_getMaxNumberOfSymbols(cValue)
		set(value) = zxing_ReaderOptions_setMaxNumberOfSymbols(cValue, value)

	val cValue: CValuesRef<zxing_ReaderOptions>? = zxing_ReaderOptions_new()

	@Suppress("unused")
	@OptIn(ExperimentalNativeApi::class)
	private val cleaner = createCleaner(cValue) { zxing_ReaderOptions_delete(it) }
}

@OptIn(ExperimentalForeignApi::class)
enum class Binarizer(internal val cValue: zxing_Binarizer) {
	LocalAverage(zxing_Binarizer_LocalAverage),
	GlobalHistogram(zxing_Binarizer_GlobalHistogram),
	FixedThreshold(zxing_Binarizer_FixedThreshold),
	BoolCast(zxing_Binarizer_BoolCast);

	companion object {
		fun fromCValue(cValue: zxing_Binarizer): Binarizer {
			return entries.first { it.cValue == cValue }
		}
	}
}

@OptIn(ExperimentalForeignApi::class)
enum class EanAddOnSymbol(internal val cValue: zxing_EanAddOnSymbol) {
	Ignore(zxing_EanAddOnSymbol_Ignore),
	Read(zxing_EanAddOnSymbol_Read),
	Require(zxing_EanAddOnSymbol_Require);

	companion object {
		fun fromCValue(cValue: zxing_EanAddOnSymbol): EanAddOnSymbol {
			return entries.first { it.cValue == cValue }
		}
	}
}

@OptIn(ExperimentalForeignApi::class)
enum class TextMode(internal val cValue: zxing_TextMode) {
	Plain(zxing_TextMode_Plain),
	ECI(zxing_TextMode_ECI),
	HRI(zxing_TextMode_HRI),
	Hex(zxing_TextMode_Hex),
	Escaped(zxing_TextMode_Escaped);

	companion object {
		fun fromCValue(cValue: zxing_TextMode): TextMode {
			return entries.first { it.cValue == cValue }
		}
	}
}
