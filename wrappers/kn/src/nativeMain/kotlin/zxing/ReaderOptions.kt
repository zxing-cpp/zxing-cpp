package zxing

import cnames.structs.zxing_ReaderOptions
import kotlinx.cinterop.CValuesRef
import kotlinx.cinterop.ExperimentalForeignApi
import zxing.cinterop.*
import zxing.cinterop.zxing_Binarizer.*
import zxing.cinterop.zxing_EanAddOnSymbol.*
import zxing.cinterop.zxing_TextMode.*

@OptIn(ExperimentalForeignApi::class)
class ReaderOptions(
	formats: Set<BarcodeFormat> = setOf(),
	tryHarder: Boolean = false,
	tryRotate: Boolean = false,
	tryInvert: Boolean = false,
	tryDownscale: Boolean = false,
	isPure: Boolean = false,
	binarizer: Binarizer = Binarizer.LocalAverage,
	minLineCount: Int = 2,
	maxNumberOfSymbols: Int = 0xff,
	returnErrors: Boolean = false,
	eanAddOnSymbol: EanAddOnSymbol = EanAddOnSymbol.Ignore,
	textMode: TextMode = TextMode.HRI,
) {
	var tryHarder: Boolean
		get() = zxing_ReaderOptions_getTryHarder(cOptions)
		set(value) = zxing_ReaderOptions_setTryHarder(cOptions, value)
	var tryRotate: Boolean
		get() = zxing_ReaderOptions_getTryRotate(cOptions)
		set(value) = zxing_ReaderOptions_setTryRotate(cOptions, value)
	var tryDownscale: Boolean
		get() = zxing_ReaderOptions_getTryDownscale(cOptions)
		set(value) = zxing_ReaderOptions_setTryDownscale(cOptions, value)
	var tryInvert: Boolean
		get() = zxing_ReaderOptions_getTryInvert(cOptions)
		set(value) = zxing_ReaderOptions_setTryInvert(cOptions, value)
	var isPure: Boolean
		get() = zxing_ReaderOptions_getIsPure(cOptions)
		set(value) = zxing_ReaderOptions_setIsPure(cOptions, value)
	var returnErrors: Boolean
		get() = zxing_ReaderOptions_getReturnErrors(cOptions)
		set(value) = zxing_ReaderOptions_setReturnErrors(cOptions, value)
	var binarizer: Binarizer
		get() = Binarizer.fromCValue(zxing_ReaderOptions_getBinarizer(cOptions))
		set(value) = zxing_ReaderOptions_setBinarizer(cOptions, value.cValue)
	var formats: Set<BarcodeFormat>
		get() = zxing_ReaderOptions_getFormats(cOptions).parseIntoBarcodeFormat()
		set(value) = zxing_ReaderOptions_setFormats(cOptions, value.toValue())
	var eanAddOnSymbol: EanAddOnSymbol
		get() = EanAddOnSymbol.fromCValue(zxing_ReaderOptions_getEanAddOnSymbol(cOptions))
		set(value) = zxing_ReaderOptions_setEanAddOnSymbol(cOptions, value.cValue)
	var textMode: TextMode
		get() = TextMode.fromCValue(zxing_ReaderOptions_getTextMode(cOptions))
		set(value) = zxing_ReaderOptions_setTextMode(cOptions, value.cValue)
	var minLineCount: Int
		get() = zxing_ReaderOptions_getMinLineCount(cOptions)
		set(value) = zxing_ReaderOptions_setMinLineCount(cOptions, value)
	var maxNumberOfSymbols: Int
		get() = zxing_ReaderOptions_getMaxNumberOfSymbols(cOptions)
		set(value) = zxing_ReaderOptions_setMaxNumberOfSymbols(cOptions, value)

	val cOptions: CValuesRef<zxing_ReaderOptions>? = zxing_ReaderOptions_new()

	init {
		this.formats = formats
		this.tryHarder = tryHarder
		this.tryRotate = tryRotate
		this.tryInvert = tryInvert
		this.tryDownscale = tryDownscale
		this.isPure = isPure
		this.binarizer = binarizer
		this.minLineCount = minLineCount
		this.maxNumberOfSymbols = maxNumberOfSymbols
		this.returnErrors = returnErrors
		this.eanAddOnSymbol = eanAddOnSymbol
		this.textMode = textMode
	}
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
