/*
* Copyright 2023 siiky
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "zxing-c.h"

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

static char* copy(std::string_view sv)
{
	auto ret = (char*)malloc(sv.size() + 1);
	if (ret) {
		strncpy(ret, sv.data(), sv.size());
		ret[sv.size()] = '\0';
	}
	return ret;
}

static uint8_t* copy(const ByteArray& ba, int* len)
{
	*len = Size(ba);

	auto ret = (uint8_t*)malloc(*len + 1);
	if (ret)
		memcpy(ret, ba.data(), *len);
	else
		*len = 0;

	return ret;
}

static std::tuple<Results, bool> ReadBarcodesAndSetLastError(const zxing_ImageView* iv, const zxing_ReaderOptions* opts,
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

zxing_ImageView* zxing_ImageView_new(const uint8_t* data, int width, int height, zxing_ImageFormat format, int rowStride,
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

zxing_ImageView* zxing_ImageView_new_checked(const uint8_t* data, int size, int width, int height, zxing_ImageFormat format,
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

void zxing_ImageView_delete(zxing_ImageView* iv)
{
	delete iv;
}

void zxing_ImageView_crop(zxing_ImageView* iv, int left, int top, int width, int height)
{
	*iv = iv->cropped(left, top, width, height);
}

void zxing_ImageView_rotate(zxing_ImageView* iv, int degree)
{
	*iv = iv->rotated(degree);
}

/*
 * ZXing/BarcodeFormat.h
 */

zxing_BarcodeFormats zxing_BarcodeFormatsFromString(const char* str)
{
	if (!str)
		return {};
	try {
		auto format = BarcodeFormatsFromString(str);
		return static_cast<zxing_BarcodeFormats>(transmute_cast<BarcodeFormat>(format));
	} catch (std::exception& e) {
		lastErrorMsg = e.what();
	} catch (...) {
		lastErrorMsg = "Unknown error";
	}

	return zxing_BarcodeFormat_Invalid;
}

zxing_BarcodeFormat zxing_BarcodeFormatFromString(const char* str)
{
	zxing_BarcodeFormat res = zxing_BarcodeFormatsFromString(str);
	return BitHacks::CountBitsSet(res) == 1 ? res : zxing_BarcodeFormat_Invalid;
}

char* zxing_BarcodeFormatToString(zxing_BarcodeFormat format)
{
	return copy(ToString(static_cast<BarcodeFormat>(format)));
}

/*
 * ZXing/ReaderOptions.h
 */

zxing_ReaderOptions* zxing_ReaderOptions_new()
{
	return new ReaderOptions();
}

void zxing_ReaderOptions_delete(zxing_ReaderOptions* opts)
{
	delete opts;
}

#define ZX_PROPERTY(TYPE, GETTER, SETTER) \
	TYPE zxing_ReaderOptions_get##SETTER(const zxing_ReaderOptions* opts) { return opts->GETTER(); } \
	void zxing_ReaderOptions_set##SETTER(zxing_ReaderOptions* opts, TYPE val) { opts->set##SETTER(val); }

ZX_PROPERTY(bool, tryHarder, TryHarder)
ZX_PROPERTY(bool, tryRotate, TryRotate)
ZX_PROPERTY(bool, tryInvert, TryInvert)
ZX_PROPERTY(bool, tryDownscale, TryDownscale)
ZX_PROPERTY(bool, isPure, IsPure)
ZX_PROPERTY(bool, returnErrors, ReturnErrors)
ZX_PROPERTY(int, minLineCount, MinLineCount)
ZX_PROPERTY(int, maxNumberOfSymbols, MaxNumberOfSymbols)

void zxing_ReaderOptions_setFormats(zxing_ReaderOptions* opts, zxing_BarcodeFormats formats)
{
	opts->setFormats(static_cast<BarcodeFormat>(formats));
}

zxing_BarcodeFormats zxing_ReaderOptions_getFormats(const zxing_ReaderOptions* opts)
{
	auto v = opts->formats();
	return transmute_cast<zxing_BarcodeFormats>(v);
}

#define ZX_ENUM_PROPERTY(TYPE, GETTER, SETTER) \
	zxing_##TYPE zxing_ReaderOptions_get##SETTER(const zxing_ReaderOptions* opts) { return static_cast<zxing_##TYPE>(opts->GETTER()); } \
	void zxing_ReaderOptions_set##SETTER(zxing_ReaderOptions* opts, zxing_##TYPE val) { opts->set##SETTER(static_cast<TYPE>(val)); }

ZX_ENUM_PROPERTY(Binarizer, binarizer, Binarizer)
ZX_ENUM_PROPERTY(EanAddOnSymbol, eanAddOnSymbol, EanAddOnSymbol)
ZX_ENUM_PROPERTY(TextMode, textMode, TextMode)

/*
 * ZXing/Result.h
 */

char* zxing_ContentTypeToString(zxing_ContentType type)
{
	return copy(ToString(static_cast<ContentType>(type)));
}

char* zxing_PositionToString(zxing_Position position)
{
	return copy(ToString(transmute_cast<Position>(position)));
}


bool zxing_Barcode_isValid(const zxing_Barcode* barcode)
{
	return barcode != NULL && barcode->isValid();
}

char* zxing_Barcode_errorMsg(const zxing_Barcode* barcode)
{
	return copy(ToString(barcode->error()));
}

uint8_t* zxing_Barcode_bytes(const zxing_Barcode* barcode, int* len)
{
	return copy(barcode->bytes(), len);
}

uint8_t* zxing_Barcode_bytesECI(const zxing_Barcode* barcode, int* len)
{
	return copy(barcode->bytesECI(), len);
}

#define ZX_GETTER(TYPE, GETTER, TRANS) \
	TYPE zxing_Barcode_##GETTER(const zxing_Barcode* barcode) { return static_cast<TYPE>(TRANS(barcode->GETTER())); }

ZX_GETTER(zxing_BarcodeFormat, format,)
ZX_GETTER(zxing_ContentType, contentType,)
ZX_GETTER(char*, text, copy)
ZX_GETTER(char*, ecLevel, copy)
ZX_GETTER(char*, symbologyIdentifier, copy)
ZX_GETTER(zxing_Position, position, transmute_cast<zxing_Position>)

ZX_GETTER(int, orientation,)
ZX_GETTER(bool, hasECI,)
ZX_GETTER(bool, isInverted,)
ZX_GETTER(bool, isMirrored,)
ZX_GETTER(int, lineCount,)


/*
 * ZXing/ReadBarcode.h
 */

zxing_Barcode* zxing_ReadBarcode(const zxing_ImageView* iv, const zxing_ReaderOptions* opts)
{
	auto [res, ok] = ReadBarcodesAndSetLastError(iv, opts, 1);
	return !res.empty() ? new Result(std::move(res.front())) : NULL;
}

zxing_Barcodes* zxing_ReadBarcodes(const zxing_ImageView* iv, const zxing_ReaderOptions* opts)
{
	auto [res, ok] = ReadBarcodesAndSetLastError(iv, opts, 0);
	return !res.empty() || ok ? new Results(std::move(res)) : NULL;
}

void zxing_Barcode_delete(zxing_Barcode* barcode)
{
	delete barcode;
}

void zxing_Barcodes_delete(zxing_Barcodes* barcodes)
{
	delete barcodes;
}

int zxing_Barcodes_size(const zxing_Barcodes* barcodes)
{
	return barcodes ? Size(*barcodes) : 0;
}

const zxing_Barcode* zxing_Barcodes_at(const zxing_Barcodes* barcodes, int i)
{
	if (!barcodes || i < 0 || i >= Size(*barcodes))
		return NULL;
	return &(*barcodes)[i];
}

zxing_Barcode* zxing_Barcodes_move(zxing_Barcodes* barcodes, int i)
{
	if (!barcodes || i < 0 || i >= Size(*barcodes))
		return NULL;

	return new Result(std::move((*barcodes)[i]));
}

char* zxing_LastErrorMsg()
{
	if (lastErrorMsg.empty())
		return NULL;

	return copy(std::exchange(lastErrorMsg, {}));
}

void zxing_free(void* ptr)
{
	free(ptr);
}

} // extern "C"
