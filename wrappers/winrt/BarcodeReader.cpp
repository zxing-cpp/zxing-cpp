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

#include "BarcodeReader.h"
#include "StringCodecs.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "DecodeHints.h"
#include "ReadResult.h"
#include "BarcodeFormat.h"

#include <wrl.h>
#include <MemoryBuffer.h>

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;

namespace ZXing {
namespace BarcodeScanner {

//static int CODE_PAGES[] = {
//	28591,	// ISO-8859-1	// default to latin1 if unknown
//	437,	// CP437
//	28591,	// ISO-8859-1
//	28592,	// ISO-8859-2
//	28593,	// ISO-8859-3
//	28594,	// ISO-8859-4
//	28595,	// ISO-8859-5
//	28596,	// ISO-8859-6
//	28597,	// ISO-8859-7
//	28598,	// ISO-8859-8
//	28599,	// ISO-8859-9
//	28600,	// ISO-8859-10
//	28601,	// ISO-8859-11
//	28603,	// ISO-8859-13
//	28604,	// ISO-8859-14
//	28605,	// ISO-8859-15
//	28606,	// ISO-8859-16
//	932,	// Shift_JIS
//	1250,	// CP1250
//	1251,	// CP1251
//	1252,	// CP1252
//	1256,	// CP1256
//	1201,	// UnicodeBig
//	65001,	// UTF-8
//	20127,	// ASCII
//	950,	// BIG5
//	936,	// GB2312
//	54936,	// GB18030
//	20932,	// EUC-JP
//	51949,	// EUC-KR
//};
//
//static_assert(std::extent<decltype(CODE_PAGES)>::value == (int)CharacterSet::CharsetCount, "CHARSET_NAMES out of sync");

class InternalStringDecoder : public StringCodecs
{
public:
	InternalStringDecoder(StringConverter^ conv) : m_converter(conv) {}

	virtual String toUnicode(const uint8_t* bytes, size_t length, CharacterSet codec) const override
	{
		if (codec == CharacterSet::ISO8859_1) {
			return String::FromLatin1(bytes, length);
		}
		else if (codec == CharacterSet::UTF8) {
			return String((const char*)bytes, length);
		}

		auto str = m_converter->Invoke(ref new Platform::Array<uint8>((uint8*)bytes, (unsigned)length), (CharEncoding)(int)codec);
		if (!str->IsEmpty())
		{
			return String(str->Begin(), str->End());
		}
		return String();
	}

	virtual CharacterSet defaultEncoding() const override
	{
		return CharacterSet::ISO8859_1;
	}

private:
	StringConverter^ m_converter;
};

BarcodeReader::BarcodeReader(StringConverter^ codec, bool tryHarder)
{
	m_decoder = std::make_shared<InternalStringDecoder>(codec);

	DecodeHints hints;
	hints.setShouldTryHarder(tryHarder);
	hints.setShouldTryRotate(tryHarder);
	m_reader.reset(new MultiFormatReader(hints, m_decoder));
}

BarcodeReader::~BarcodeReader()
{
}

static std::shared_ptr<BinaryBitmap>
CreateBinaryBitmap(SoftwareBitmap^ bitmap)
{
	auto inBuffer = bitmap->LockBuffer(BitmapBufferAccessMode::Read);
	auto inMemRef = inBuffer->CreateReference();
	ComPtr<IMemoryBufferByteAccess> inBufferAccess;
	if (SUCCEEDED(ComPtr<IUnknown>(reinterpret_cast<IUnknown*>(inMemRef)).As(&inBufferAccess)))
	{
		BYTE* inBytes = nullptr;
		UINT32 inCapacity = 0;
		inBufferAccess->GetBuffer(&inBytes, &inCapacity);

		std::shared_ptr<GenericLuminanceSource> luminance;
		switch (bitmap->BitmapPixelFormat)
		{
		case BitmapPixelFormat::Gray8:
			luminance = std::make_shared<GenericLuminanceSource>(bitmap->PixelWidth, bitmap->PixelHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride);
			break;
		case BitmapPixelFormat::Bgra8:
			luminance = std::make_shared<GenericLuminanceSource>(bitmap->PixelWidth, bitmap->PixelHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride, 4, 2, 1, 0);
			break;
		case BitmapPixelFormat::Rgba8:
			luminance = std::make_shared<GenericLuminanceSource>(bitmap->PixelWidth, bitmap->PixelHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride, 4, 0, 1, 2);
			break;
		default:
			throw std::runtime_error("Unsupported format");
		}
		return std::make_shared<HybridBinarizer>(luminance);
	}
	else
	{
		throw std::runtime_error("Failed to read bitmap's data");
	}
}

static Platform::String^ ToPlatformString(const String& str)
{
	auto tmp = str.toWString();
	return ref new Platform::String(tmp.c_str(), (unsigned)tmp.length());
}

ReadResult^
BarcodeReader::Read(SoftwareBitmap^ bitmap)
{
	auto binImg = CreateBinaryBitmap(bitmap);
	auto result = m_reader->read(*binImg);
	if (result.isValid()) {
		return ref new ReadResult(ToPlatformString(ZXing::ToString(result.format())), ToPlatformString(result.text()));
	}
	return nullptr;
}

} // BarcodeScanner
} // ZXing
