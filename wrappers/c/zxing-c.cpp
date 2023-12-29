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
#include <utility>

using namespace ZXing;

static thread_local std::string lastErrorMsg;

static char* copy(std::string_view sv)
{
	auto ret = (char*)malloc(sv.size() + 1);
	if (ret) {
		strncpy(ret, sv.data(), sv.size());
		ret[sv.size()] = '\0';
	}
	return ret;
}

static Results ReadBarcodesAndSetLastError(const zxing_ImageView* iv, const zxing_ReaderOptions* opts, int maxSymbols)
{
	try {
		if (iv) {
			auto o = opts ? *opts : ReaderOptions{};
			if (maxSymbols)
				o.setMaxNumberOfSymbols(maxSymbols);
			return ReadBarcodes(*iv, o);
		} else
			lastErrorMsg = "ImageView param is NULL";
	} catch (std::exception& e) {
		lastErrorMsg = e.what();
	} catch (...) {
		lastErrorMsg = "Unknown error";
	}

	return {};
}

extern "C" {
/*
 * ZXing/ImageView.h
 */

zxing_ImageView* zxing_ImageView_new(const uint8_t* data, int width, int height, zxing_ImageFormat format, int rowStride,
									 int pixStride)
{
	ImageFormat cppformat = static_cast<ImageFormat>(format);
	return new ImageView(data, width, height, cppformat, rowStride, pixStride);
}

void zxing_ImageView_delete(zxing_ImageView* iv)
{
	delete iv;
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
		return static_cast<zxing_BarcodeFormats>(*reinterpret_cast<BarcodeFormat*>(&format));
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

void zxing_ReaderOptions_setTryHarder(zxing_ReaderOptions* opts, bool tryHarder)
{
	opts->setTryHarder(tryHarder);
}

void zxing_ReaderOptions_setTryRotate(zxing_ReaderOptions* opts, bool tryRotate)
{
	opts->setTryRotate(tryRotate);
}

void zxing_ReaderOptions_setTryInvert(zxing_ReaderOptions* opts, bool tryInvert)
{
	opts->setTryInvert(tryInvert);
}

void zxing_ReaderOptions_setTryDownscale(zxing_ReaderOptions* opts, bool tryDownscale)
{
	opts->setTryDownscale(tryDownscale);
}

void zxing_ReaderOptions_setIsPure(zxing_ReaderOptions* opts, bool isPure)
{
	opts->setIsPure(isPure);
}

void zxing_ReaderOptions_setReturnErrors(zxing_ReaderOptions* opts, bool returnErrors)
{
	opts->setReturnErrors(returnErrors);
}

void zxing_ReaderOptions_setFormats(zxing_ReaderOptions* opts, zxing_BarcodeFormats formats)
{
	opts->setFormats(static_cast<BarcodeFormat>(formats));
}

void zxing_ReaderOptions_setBinarizer(zxing_ReaderOptions* opts, zxing_Binarizer binarizer)
{
	opts->setBinarizer(static_cast<Binarizer>(binarizer));
}

void zxing_ReaderOptions_setEanAddOnSymbol(zxing_ReaderOptions* opts, zxing_EanAddOnSymbol eanAddOnSymbol)
{
	opts->setEanAddOnSymbol(static_cast<EanAddOnSymbol>(eanAddOnSymbol));
}

void zxing_ReaderOptions_setTextMode(zxing_ReaderOptions* opts, zxing_TextMode textMode)
{
	opts->setTextMode(static_cast<TextMode>(textMode));
}

void zxing_ReaderOptions_setMaxNumberOfSymbols(zxing_ReaderOptions* opts, int n)
{
	opts->setMaxNumberOfSymbols(n);
}

/*
 * ZXing/Result.h
 */

char* zxing_ContentTypeToString(zxing_ContentType type)
{
	return copy(ToString(static_cast<ContentType>(type)));
}

bool zxing_Result_isValid(const zxing_Result* result)
{
	return result != NULL && result->isValid();
}

char* zxing_Result_errorMsg(const zxing_Result* result)
{
	return copy(ToString(result->error()));
}

zxing_BarcodeFormat zxing_Result_format(const zxing_Result* result)
{
	return static_cast<zxing_BarcodeFormat>(result->format());
}

zxing_ContentType zxing_Result_contentType(const zxing_Result* result)
{
	return static_cast<zxing_ContentType>(result->contentType());
}

uint8_t* zxing_Result_bytes(const zxing_Result* result, int* len)
{
	*len = Size(result->bytes());

	auto ret = (uint8_t*)malloc(*len + 1);
	if (ret)
		memcpy(ret, result->bytes().data(), *len);
	else
		*len = 0;

	return ret;
}

char* zxing_Result_text(const zxing_Result* result)
{
	return copy(result->text());
}

char* zxing_Result_ecLevel(const zxing_Result* result)
{
	return copy(result->ecLevel());
}

char* zxing_Result_symbologyIdentifier(const zxing_Result* result)
{
	return copy(result->symbologyIdentifier());
}

int zxing_Result_orientation(const zxing_Result* result)
{
	return result->orientation();
}

bool zxing_Result_isInverted(const zxing_Result* result)
{
	return result->isInverted();
}

bool zxing_Result_isMirrored(const zxing_Result* result)
{
	return result->isMirrored();
}

/*
 * ZXing/ReadBarcode.h
 */

zxing_Result* zxing_ReadBarcode(const zxing_ImageView* iv, const zxing_ReaderOptions* opts)
{
	auto res = ReadBarcodesAndSetLastError(iv, opts, 1);
	return !res.empty() ? new Result(std::move(res.front())) : NULL;
}

zxing_Results* zxing_ReadBarcodes(const zxing_ImageView* iv, const zxing_ReaderOptions* opts)
{
	auto res = ReadBarcodesAndSetLastError(iv, opts, 0);
	return !res.empty() ? new Results(std::move(res)) : NULL;
}

void zxing_Result_delete(zxing_Result* result)
{
	delete result;
}

void zxing_Results_delete(zxing_Results* results)
{
	delete results;
}

int zxing_Results_size(const zxing_Results* results)
{
	return results ? Size(*results) : 0;
}

const zxing_Result* zxing_Results_at(const zxing_Results* results, int i)
{
	if (!results || i < 0 || i >= Size(*results))
		return NULL;
	return &(*results)[i];
}

char* zxing_LastErrorMsg()
{
	if (lastErrorMsg.empty())
		return NULL;

	return copy(std::exchange(lastErrorMsg, {}));
}

} // extern "C"
