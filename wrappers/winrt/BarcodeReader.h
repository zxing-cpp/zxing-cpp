#pragma once
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

#include <memory>

namespace ZXing {

class MultiFormatReader;

namespace BarcodeScanner {

ref class ReadResult;

public enum class CharEncoding
{
	Unknown,
	Cp437,
	ISO8859_1,
	ISO8859_2,
	ISO8859_3,
	ISO8859_4,
	ISO8859_5,
	ISO8859_6,
	ISO8859_7,
	ISO8859_8,
	ISO8859_9,
	ISO8859_10,
	ISO8859_11,
	ISO8859_13,
	ISO8859_14,
	ISO8859_15,
	ISO8859_16,
	Shift_JIS,
	Cp1250,
	Cp1251,
	Cp1252,
	Cp1256,
	UnicodeBig,
	UTF8,
	ASCII,
	Big5,
	GB2312,
	GB18030,
	EUC_JP,
	EUC_KR,
};

public delegate Platform::String^ StringConverter(const Platform::Array<uint8>^, CharEncoding charset);
class InternalStringDecoder;

public ref class BarcodeReader sealed
{
public:
	BarcodeReader(StringConverter^ codec, bool tryHarder);

	ReadResult^ Read(Windows::Graphics::Imaging::SoftwareBitmap^ bitmap);

private:
	~BarcodeReader();

	std::unique_ptr<MultiFormatReader> m_reader;
	std::shared_ptr<InternalStringDecoder> m_decoder;
};

} // BarcodeScanner
} // ZXing
