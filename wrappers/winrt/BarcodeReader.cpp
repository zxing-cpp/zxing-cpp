/*
* Copyright 2016 Nu-book Inc.
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

#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif

#include "BarcodeReader.h"

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "MultiFormatReader.h"
#include "ReadResult.h"
#include "Result.h"

#include <algorithm>
#include <MemoryBuffer.h>
#include <stdexcept>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;

namespace ZXing {

BarcodeReader::BarcodeReader(bool tryHarder, bool tryRotate, const Platform::Array<BarcodeType>^ types)
{
	DecodeHints hints;
	hints.setShouldTryHarder(tryHarder);
	hints.setShouldTryRotate(tryRotate);

	if (types != nullptr && types->Length > 0) {
		std::vector<BarcodeFormat> barcodeFormats;
		for (BarcodeType type : types) {
			barcodeFormats.emplace_back(BarcodeReader::ConvertRuntimeToNative(type));
		}
		hints.setPossibleFormats(barcodeFormats);
	}

	m_reader.reset(new MultiFormatReader(hints));
}

BarcodeReader::BarcodeReader(bool tryHarder, bool tryRotate)
{
	DecodeHints hints;
	hints.setShouldTryHarder(tryHarder);
	hints.setShouldTryRotate(tryRotate);
	m_reader.reset(new MultiFormatReader(hints));
}

BarcodeReader::BarcodeReader(bool tryHarder)
{
	DecodeHints hints;
	hints.setShouldTryHarder(tryHarder);
	hints.setShouldTryRotate(tryHarder);
	m_reader.reset(new MultiFormatReader(hints));
}

BarcodeReader::~BarcodeReader()
{
}

BarcodeFormat BarcodeReader::ConvertRuntimeToNative(BarcodeType type)
{
	switch (type) {
	case BarcodeType::AZTEC:
		return BarcodeFormat::AZTEC;
	case BarcodeType::CODABAR:
		return BarcodeFormat::CODABAR;
	case BarcodeType::CODE_128:
		return BarcodeFormat::CODE_128;
	case BarcodeType::CODE_39:
		return BarcodeFormat::CODE_39;
	case BarcodeType::CODE_93:
		return BarcodeFormat::CODE_93;
	case BarcodeType::DATA_MATRIX:
		return BarcodeFormat::DATA_MATRIX;
	case BarcodeType::EAN_13:
		return BarcodeFormat::EAN_13;
	case BarcodeType::EAN_8:
		return BarcodeFormat::EAN_8;
	case BarcodeType::ITF:
		return BarcodeFormat::ITF;
	case BarcodeType::MAXICODE:
		return BarcodeFormat::MAXICODE;
	case BarcodeType::PDF_417:
		return BarcodeFormat::PDF_417;
	case BarcodeType::QR_CODE:
		return BarcodeFormat::QR_CODE;
	case BarcodeType::RSS_14:
		return BarcodeFormat::RSS_14;
	case BarcodeType::RSS_EXPANDED:
		return BarcodeFormat::RSS_EXPANDED;
	case BarcodeType::UPC_A:
		return BarcodeFormat::UPC_A;
	case BarcodeType::UPC_E:
		return BarcodeFormat::UPC_E;
	case BarcodeType::UPC_EAN_EXTENSION:
		return BarcodeFormat::UPC_EAN_EXTENSION;
	default:
		std::wstring typeAsString = type.ToString()->Begin();
		throw std::invalid_argument("Unknown Barcode Format: "
			+ std::string( typeAsString.begin(), typeAsString.end()));
	}
}

static std::shared_ptr<BinaryBitmap>
CreateBinaryBitmap(SoftwareBitmap^ bitmap, int cropWidth, int cropHeight)
{
	cropWidth = cropWidth <= 0 ? bitmap->PixelWidth : std::min(bitmap->PixelWidth, cropWidth);
	cropHeight = cropHeight <= 0 ? bitmap->PixelHeight : std::min(bitmap->PixelHeight, cropHeight);
	int cropLeft = (bitmap->PixelWidth - cropWidth) / 2;
	int cropTop = (bitmap->PixelHeight - cropHeight) / 2;

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
			luminance = std::make_shared<GenericLuminanceSource>(cropLeft, cropTop, cropWidth, cropHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride);
			break;
		case BitmapPixelFormat::Bgra8:
			luminance = std::make_shared<GenericLuminanceSource>(cropLeft, cropTop, cropWidth, cropHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride, 4, 2, 1, 0);
			break;
		case BitmapPixelFormat::Rgba8:
			luminance = std::make_shared<GenericLuminanceSource>(cropLeft, cropTop, cropWidth, cropHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride, 4, 0, 1, 2);
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

static Platform::String^ ToPlatformString(const std::wstring& str)
{
	return ref new Platform::String(str.c_str(), (unsigned)str.length());
}

static Platform::String^ ToPlatformString(const std::string& str)
{
	auto ptr = (const uint8_t*)str.data();
	return ToPlatformString(std::wstring(ptr, ptr + str.length()));
}

ReadResult^
BarcodeReader::Read(SoftwareBitmap^ bitmap, int cropWidth, int cropHeight)
{
	try {
		auto binImg = CreateBinaryBitmap(bitmap, cropWidth, cropHeight);
		auto result = m_reader->read(*binImg);
		if (result.isValid()) {
			return ref new ReadResult(ToPlatformString(ZXing::ToString(result.format())), ToPlatformString(result.text()));
		}
	}
	catch (const std::exception& e) {
		OutputDebugStringA(e.what());
	}
	catch (...) {
	}
	return nullptr;
}

} // ZXing
