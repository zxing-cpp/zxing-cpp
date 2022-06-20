/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#pragma warning(disable : 4996)
#endif

#include "BarcodeReader.h"

#define ZX_USE_UTF8 1 // silence deprecation warning in Result.h

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "ReadBarcode.h"
#include "ReadResult.h"
#include "TextUtfEncoding.h"

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
	init(tryHarder, tryRotate, types);
}

BarcodeReader::BarcodeReader(bool tryHarder, bool tryRotate)
{
	init(tryHarder, tryRotate, nullptr);
}

BarcodeReader::BarcodeReader(bool tryHarder)
{
	init(tryHarder, tryHarder, nullptr);
}

void
BarcodeReader::init(bool tryHarder, bool tryRotate, const Platform::Array<BarcodeType>^ types)
{
	m_hints.reset(new DecodeHints());
	m_hints->setTryHarder(tryHarder);
	m_hints->setTryRotate(tryRotate);

	if (types != nullptr && types->Length > 0) {
		BarcodeFormats barcodeFormats;
		for (BarcodeType type : types) {
			barcodeFormats |= BarcodeReader::ConvertRuntimeToNative(type);
		}
		m_hints->setFormats(barcodeFormats);
	}
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
	case BarcodeType::MICRO_QR_CODE:
		return BarcodeFormat::MicroQRCode;
	case BarcodeType::RSS_14:
		return BarcodeFormat::RSS_14;
	case BarcodeType::RSS_EXPANDED:
		return BarcodeFormat::RSS_EXPANDED;
	case BarcodeType::UPC_A:
		return BarcodeFormat::UPC_A;
	case BarcodeType::UPC_E:
		return BarcodeFormat::UPC_E;
	default:
		std::wstring typeAsString = type.ToString()->Begin();
		throw std::invalid_argument("Unknown Barcode Type: " + TextUtfEncoding::ToUtf8(typeAsString));
	}
}

BarcodeType BarcodeReader::ConvertNativeToRuntime(BarcodeFormat format)
{
	switch (format) {
	case BarcodeFormat::AZTEC:
		return BarcodeType::AZTEC;
	case BarcodeFormat::CODABAR:
		return BarcodeType::CODABAR;
	case BarcodeFormat::CODE_128:
		return BarcodeType::CODE_128;
	case BarcodeFormat::CODE_39:
		return BarcodeType::CODE_39;
	case BarcodeFormat::CODE_93:
		return BarcodeType::CODE_93;
	case BarcodeFormat::DATA_MATRIX:
		return BarcodeType::DATA_MATRIX;
	case BarcodeFormat::EAN_13:
		return BarcodeType::EAN_13;
	case BarcodeFormat::EAN_8:
		return BarcodeType::EAN_8;
	case BarcodeFormat::ITF:
		return BarcodeType::ITF;
	case BarcodeFormat::MAXICODE:
		return BarcodeType::MAXICODE;
	case BarcodeFormat::PDF_417:
		return BarcodeType::PDF_417;
	case BarcodeFormat::QR_CODE:
		return BarcodeType::QR_CODE;
	case BarcodeFormat::MicroQRCode:
		return BarcodeType::MICRO_QR_CODE;
	case BarcodeFormat::RSS_14:
		return BarcodeType::RSS_14;
	case BarcodeFormat::RSS_EXPANDED:
		return BarcodeType::RSS_EXPANDED;
	case BarcodeFormat::UPC_A:
		return BarcodeType::UPC_A;
	case BarcodeFormat::UPC_E:
		return BarcodeType::UPC_E;
	default:
		throw std::invalid_argument("Unknown Barcode Format ");
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
		cropWidth = cropWidth <= 0 ? bitmap->PixelWidth : std::min(bitmap->PixelWidth, cropWidth);
		cropHeight = cropHeight <= 0 ? bitmap->PixelHeight : std::min(bitmap->PixelHeight, cropHeight);
		int cropLeft = (bitmap->PixelWidth - cropWidth) / 2;
		int cropTop = (bitmap->PixelHeight - cropHeight) / 2;

		auto inBuffer = bitmap->LockBuffer(BitmapBufferAccessMode::Read);
		auto inMemRef = inBuffer->CreateReference();
		ComPtr<IMemoryBufferByteAccess> inBufferAccess;

		if (SUCCEEDED(ComPtr<IUnknown>(reinterpret_cast<IUnknown*>(inMemRef)).As(&inBufferAccess))) {
			BYTE* inBytes = nullptr;
			UINT32 inCapacity = 0;
			inBufferAccess->GetBuffer(&inBytes, &inCapacity);

			ImageFormat fmt = ImageFormat::None;
			switch (bitmap->BitmapPixelFormat)
			{
			case BitmapPixelFormat::Gray8: fmt = ImageFormat::Lum; break;
			case BitmapPixelFormat::Bgra8: fmt = ImageFormat::BGRX; break;
			case BitmapPixelFormat::Rgba8: fmt = ImageFormat::RGBX; break;
			default:
				throw std::runtime_error("Unsupported BitmapPixelFormat");
			}

			auto img = ImageView(inBytes, bitmap->PixelWidth, bitmap->PixelHeight, fmt, inBuffer->GetPlaneDescription(0).Stride)
						   .cropped(cropLeft, cropTop, cropWidth, cropHeight);

			auto result = ReadBarcode(img, *m_hints);
			if (result.isValid()) {
				return ref new ReadResult(ToPlatformString(ZXing::ToString(result.format())), ToPlatformString(result.utf16()), ConvertNativeToRuntime(result.format()));
			}
		} else {
			throw std::runtime_error("Failed to read bitmap's data");
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
