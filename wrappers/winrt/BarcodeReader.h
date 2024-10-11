/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ReaderOptions.h"

#include <memory>

namespace ZXing {

public enum class BarcodeType : int {
	AZTEC,
	CODABAR,
	CODE_39,
	CODE_93,
	CODE_128,
	DATA_MATRIX,
	DX_FILM_EDGE,
	EAN_8,
	EAN_13,
	ITF,
	MAXICODE,
	PDF_417,
	QR_CODE,
	MICRO_QR_CODE,
	RMQR_CODE,
	RSS_14,
	RSS_EXPANDED,
	DATA_BAR_LIMITED,
	UPC_A,
	UPC_E
};

ref class ReadResult;

public ref class BarcodeReader sealed
{
public:
	BarcodeReader(bool tryHarder, bool tryRotate, const Platform::Array<BarcodeType>^ types);
	BarcodeReader(bool tryHarder, bool tryRotate);
	BarcodeReader(bool tryHarder);

	ReadResult^ Read(Windows::Graphics::Imaging::SoftwareBitmap^ bitmap, int cropWidth, int cropHeight);

private:
	~BarcodeReader();

	void init(bool tryHarder, bool tryRotate, const Platform::Array<BarcodeType>^ types);

	static BarcodeFormat ConvertRuntimeToNative(BarcodeType type);
	static BarcodeType ConvertNativeToRuntime(BarcodeFormat format);

	std::unique_ptr<ReaderOptions> m_opts;
};

} // ZXing
