/*
* Copyright 2023 siiky
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingC.h"

#include "ReadBarcode.h"

#include <cstdlib>
#include <exception>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

using namespace ZXing;

static thread_local std::string lastErrorMsg;

template<typename R, typename T> R transmute_cast(const T& v)
{
	static_assert(sizeof(T) == sizeof(R));
	return *(const R*)(&v);
}

template<typename C, typename P = typename C::pointer>
P copy(const C& c)
{
	auto ret = (P)malloc(c.size() + 1);
	if (ret) {
		memcpy(ret, c.data(), c.size());
		ret[c.size()] = 0;
	}
	return ret;
}

static uint8_t* copy(const ByteArray& ba, int* len)
{
	// for convencience and as a safety measure, we NULL terminate even byte arrays
	auto ret = copy(ba);
	if (len)
		*len = ret ? Size(ba) : 0;
	return ret;
}

static std::tuple<Results, bool> ReadBarcodesAndSetLastError(const ZXing_ImageView* iv, const ZXing_ReaderOptions* opts,
															 int maxSymbols)
{
	try {
		if (iv) {
			auto o = opts ? *opts : ReaderOptions{};
			if (maxSymbols)
				o.setMaxNumberOfSymbols(maxSymbols);
			return {ReadBarcodes(*iv, o), true};
		} else
			lastErrorMsg = "ImageView param is NULL";
	} catch (std::exception& e) {
		lastErrorMsg = e.what();
	} catch (...) {
		lastErrorMsg = "Unknown error";
	}

	return {Results{}, false};
}

extern "C" {
/*
 * ZXing/ImageView.h
 */

ZXing_ImageView* ZXing_ImageView_new(const uint8_t* data, int width, int height, ZXing_ImageFormat format, int rowStride,
									 int pixStride)
{
	ImageFormat cppformat = static_cast<ImageFormat>(format);
	try {
		return new ImageView(data, width, height, cppformat, rowStride, pixStride);
	} catch (std::exception& e) {
		lastErrorMsg = e.what();
	}
	return NULL;
}

ZXing_ImageView* ZXing_ImageView_new_checked(const uint8_t* data, int size, int width, int height, ZXing_ImageFormat format,
											 int rowStride, int pixStride)
{
	ImageFormat cppformat = static_cast<ImageFormat>(format);
	try {
		return new ImageView(data, size, width, height, cppformat, rowStride, pixStride);
	} catch (std::exception& e) {
		lastErrorMsg = e.what();
	}
	return NULL;
}

void ZXing_ImageView_delete(ZXing_ImageView* iv)
{
	delete iv;
}

void ZXing_ImageView_crop(ZXing_ImageView* iv, int left, int top, int width, int height)
{
	*iv = iv->cropped(left, top, width, height);
}

void ZXing_ImageView_rotate(ZXing_ImageView* iv, int degree)
{
	*iv = iv->rotated(degree);
}

/*
 * ZXing/BarcodeFormat.h
 */

ZXing_BarcodeFormats ZXing_BarcodeFormatsFromString(const char* str)
{
	if (!str)
		return {};
	try {
		auto format = BarcodeFormatsFromString(str);
		return static_cast<ZXing_BarcodeFormats>(transmute_cast<BarcodeFormat>(format));
	} catch (std::exception& e) {
		lastErrorMsg = e.what();
	} catch (...) {
		lastErrorMsg = "Unknown error";
	}

	return ZXing_BarcodeFormat_Invalid;
}

ZXing_BarcodeFormat ZXing_BarcodeFormatFromString(const char* str)
{
	ZXing_BarcodeFormat res = ZXing_BarcodeFormatsFromString(str);
	return BitHacks::CountBitsSet(res) == 1 ? res : ZXing_BarcodeFormat_Invalid;
}

char* ZXing_BarcodeFormatToString(ZXing_BarcodeFormat format)
{
	return copy(ToString(static_cast<BarcodeFormat>(format)));
}

/*
 * ZXing/ReaderOptions.h
 */

ZXing_ReaderOptions* ZXing_ReaderOptions_new()
{
	return new ReaderOptions();
}

void ZXing_ReaderOptions_delete(ZXing_ReaderOptions* opts)
{
	delete opts;
}

#define ZX_PROPERTY(TYPE, GETTER, SETTER) \
	TYPE ZXing_ReaderOptions_get##SETTER(const ZXing_ReaderOptions* opts) { return opts->GETTER(); } \
	void ZXing_ReaderOptions_set##SETTER(ZXing_ReaderOptions* opts, TYPE val) { opts->set##SETTER(val); }

ZX_PROPERTY(bool, tryHarder, TryHarder)
ZX_PROPERTY(bool, tryRotate, TryRotate)
ZX_PROPERTY(bool, tryInvert, TryInvert)
ZX_PROPERTY(bool, tryDownscale, TryDownscale)
ZX_PROPERTY(bool, isPure, IsPure)
ZX_PROPERTY(bool, returnErrors, ReturnErrors)
ZX_PROPERTY(int, minLineCount, MinLineCount)
ZX_PROPERTY(int, maxNumberOfSymbols, MaxNumberOfSymbols)

void ZXing_ReaderOptions_setFormats(ZXing_ReaderOptions* opts, ZXing_BarcodeFormats formats)
{
	opts->setFormats(static_cast<BarcodeFormat>(formats));
}

ZXing_BarcodeFormats ZXing_ReaderOptions_getFormats(const ZXing_ReaderOptions* opts)
{
	auto v = opts->formats();
	return transmute_cast<ZXing_BarcodeFormats>(v);
}

#define ZX_ENUM_PROPERTY(TYPE, GETTER, SETTER) \
	ZXing_##TYPE ZXing_ReaderOptions_get##SETTER(const ZXing_ReaderOptions* opts) { return static_cast<ZXing_##TYPE>(opts->GETTER()); } \
	void ZXing_ReaderOptions_set##SETTER(ZXing_ReaderOptions* opts, ZXing_##TYPE val) { opts->set##SETTER(static_cast<TYPE>(val)); }

ZX_ENUM_PROPERTY(Binarizer, binarizer, Binarizer)
ZX_ENUM_PROPERTY(EanAddOnSymbol, eanAddOnSymbol, EanAddOnSymbol)
ZX_ENUM_PROPERTY(TextMode, textMode, TextMode)

/*
 * ZXing/Result.h
 */

char* ZXing_ContentTypeToString(ZXing_ContentType type)
{
	return copy(ToString(static_cast<ContentType>(type)));
}

char* ZXing_PositionToString(ZXing_Position position)
{
	return copy(ToString(transmute_cast<Position>(position)));
}


bool ZXing_Barcode_isValid(const ZXing_Barcode* barcode)
{
	return barcode != NULL && barcode->isValid();
}

char* ZXing_Barcode_errorMsg(const ZXing_Barcode* barcode)
{
	return copy(ToString(barcode->error()));
}

uint8_t* ZXing_Barcode_bytes(const ZXing_Barcode* barcode, int* len)
{
	return copy(barcode->bytes(), len);
}

uint8_t* ZXing_Barcode_bytesECI(const ZXing_Barcode* barcode, int* len)
{
	return copy(barcode->bytesECI(), len);
}

#define ZX_GETTER(TYPE, GETTER, TRANS) \
	TYPE ZXing_Barcode_##GETTER(const ZXing_Barcode* barcode) { return static_cast<TYPE>(TRANS(barcode->GETTER())); }

ZX_GETTER(ZXing_BarcodeFormat, format,)
ZX_GETTER(ZXing_ContentType, contentType,)
ZX_GETTER(char*, text, copy)
ZX_GETTER(char*, ecLevel, copy)
ZX_GETTER(char*, symbologyIdentifier, copy)
ZX_GETTER(ZXing_Position, position, transmute_cast<ZXing_Position>)

ZX_GETTER(int, orientation,)
ZX_GETTER(bool, hasECI,)
ZX_GETTER(bool, isInverted,)
ZX_GETTER(bool, isMirrored,)
ZX_GETTER(int, lineCount,)


/*
 * ZXing/ReadBarcode.h
 */

ZXing_Barcode* ZXing_ReadBarcode(const ZXing_ImageView* iv, const ZXing_ReaderOptions* opts)
{
	auto [res, ok] = ReadBarcodesAndSetLastError(iv, opts, 1);
	return !res.empty() ? new Result(std::move(res.front())) : NULL;
}

ZXing_Barcodes* ZXing_ReadBarcodes(const ZXing_ImageView* iv, const ZXing_ReaderOptions* opts)
{
	auto [res, ok] = ReadBarcodesAndSetLastError(iv, opts, 0);
	return !res.empty() || ok ? new Results(std::move(res)) : NULL;
}

void ZXing_Barcode_delete(ZXing_Barcode* barcode)
{
	delete barcode;
}

void ZXing_Barcodes_delete(ZXing_Barcodes* barcodes)
{
	delete barcodes;
}

int ZXing_Barcodes_size(const ZXing_Barcodes* barcodes)
{
	return barcodes ? Size(*barcodes) : 0;
}

const ZXing_Barcode* ZXing_Barcodes_at(const ZXing_Barcodes* barcodes, int i)
{
	if (!barcodes || i < 0 || i >= Size(*barcodes))
		return NULL;
	return &(*barcodes)[i];
}

ZXing_Barcode* ZXing_Barcodes_move(ZXing_Barcodes* barcodes, int i)
{
	if (!barcodes || i < 0 || i >= Size(*barcodes))
		return NULL;

	return new Result(std::move((*barcodes)[i]));
}

char* ZXing_LastErrorMsg()
{
	if (lastErrorMsg.empty())
		return NULL;

	return copy(std::exchange(lastErrorMsg, {}));
}

void ZXing_free(void* ptr)
{
	free(ptr);
}

} // extern "C"
