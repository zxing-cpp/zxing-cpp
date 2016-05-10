/*
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "BarcodeScanner.h"
#include "StringCodecs.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "DecodeHints.h"
#include "iconv.h"

#include <windows.h>
#include <gdiplus.h>
#include <mutex>
#include <type_traits>

namespace ZXing {

namespace {

static const char* CHARSET_NAMES[] = {
	"ISO-8859-1",	// default to latin1 if unknown
	"CP437",
	"ISO-8859-1",
	"ISO-8859-2",
	"ISO-8859-3",
	"ISO-8859-4",
	"ISO-8859-5",
	"ISO-8859-6",
	"ISO-8859-7",
	"ISO-8859-8",
	"ISO-8859-9",
	"ISO-8859-10",
	"ISO-8859-11",
	"ISO-8859-13",
	"ISO-8859-14",
	"ISO-8859-15",
	"ISO-8859-16",
	"SJIS",
	"CP1250",
	"CP1251",
	"CP1252",
	"CP1256",
	"UCS-2BE",
	"UTF-8",
	"ASCII",
	"BIG5",
	"GB2312",
	"GB18030",
	"EUC-JP",
	"EUC-KR",
};

static_assert(std::extent<decltype(CHARSET_NAMES)>::value == (int)CharacterSet::CharsetCount, "CHARSET_NAMES out of sync");


class IconvCodecs : public StringCodecs
{
public:

	virtual String toUnicode(const uint8_t* bytes, size_t length, CharacterSet codec) const override
	{
		iconv_t cd = iconv_open("UTF-8", CHARSET_NAMES[(int)codec]);
		if (cd == (iconv_t)-1) {
			return String(reinterpret_cast<const char*>(bytes), length);
		}

		std::vector<char> bufOut;
		bufOut.resize(4 * length + 1);

		char *fromPtr = (char *)bytes;
		size_t nFrom = length;
		char *toPtr = bufOut.data();
		size_t nTo = bufOut.size();

		while (nFrom > 0) {
			size_t oneway = iconv(cd, &fromPtr, &nFrom, &toPtr, &nTo);
			if (oneway == (size_t)(-1)) {
				iconv_close(cd);
				return String(reinterpret_cast<const char*>(bytes), length);
			}
		}
		iconv_close(cd);
		return String(bufOut.data(), bufOut.size() - nTo);
	}
	
	virtual CharacterSet defaultEncoding() const override
	{
		return CharacterSet::ISO8859_1;
	}

};

} // anonymous

static void InitStringCodecs()
{
	StringCodecs::SetInstance(std::make_shared<IconvCodecs>());
}

BarcodeScanner::BarcodeScanner(bool tryHarder, bool tryRotate, const std::string& format)
{
	static std::once_flag s_once;
	std::call_once(s_once, InitStringCodecs);

	DecodeHints hints;
	hints.setShouldTryHarder(tryHarder);
	hints.setShouldTryRotate(tryRotate);
	if (!format.empty()) {
		BarcodeFormat f = FromString(format.c_str());
		if (f != BarcodeFormat::FORMAT_COUNT) {
			hints.setPossibleFormats({ f });
		}
	}
	_reader = std::make_shared<MultiFormatReader>(hints);
}

static std::shared_ptr<LuminanceSource>
CreateLuminanceSource(Gdiplus::Bitmap& bitmap, const Gdiplus::BitmapData& data)
{
	switch (bitmap.GetPixelFormat())
	{
	case PixelFormat24bppRGB:
		return std::make_shared<GenericLuminanceSource>(data.Width, data.Height, data.Scan0, data.Stride, 3, 2, 1, 0);
	case PixelFormat32bppARGB:
	case PixelFormat32bppRGB:
		return std::make_shared<GenericLuminanceSource>(data.Width, data.Height, data.Scan0, data.Stride, 4, 2, 1, 0);
	}
	throw std::invalid_argument("Unsupported format");
}

static std::shared_ptr<BinaryBitmap>
CreateBinaryBitmap(Gdiplus::Bitmap& bitmap)
{
	Gdiplus::BitmapData data;
	bitmap.LockBits(nullptr, Gdiplus::ImageLockModeRead, bitmap.GetPixelFormat(), &data);
	try
	{
		auto result = std::make_shared<HybridBinarizer>(CreateLuminanceSource(bitmap, data));
		bitmap.UnlockBits(&data);
		return result;
	}
	catch (...)
	{
		bitmap.UnlockBits(&data);
		throw;
	}
}

BarcodeScanner::ScanResult
BarcodeScanner::scan(Gdiplus::Bitmap& bitmap, int rotations)
{
	Result result(ErrorStatus::NotFound);
	auto binImg = CreateBinaryBitmap(bitmap);
	
	if ((rotations & Rotation0) != 0) {
		result = _reader->read(*binImg);
	}
	if (!result.isValid() && (rotations & Rotation180) != 0) {
		result = _reader->read(*binImg->rotated(180));
	}
	if (!result.isValid() && (rotations & RotationCW90) != 0) {
		result = _reader->read(*binImg->rotated(90));
	}
	if (!result.isValid() && (rotations & RotationCCW90) != 0) {
		result = _reader->read(*binImg->rotated(270));
	}
	if (result.isValid()) {
		return{ ToString(result.format()), result.text().toStdString() };
	}
	return ScanResult();
}

} // ZXing
