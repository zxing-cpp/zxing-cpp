#pragma once
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

namespace ZXing {

public ref class ReadResult sealed
{
public:
	property Platform::String^ Format {
		Platform::String^ get() {
			return m_format;
		}
	}

	property Platform::String^ Text {
		Platform::String^ get() {
			return m_text;
		}
	}

	property BarcodeType Type {
		BarcodeType get() {
			return m_barcode_type;
		}
	}

internal:
	ReadResult(Platform::String^ format, Platform::String^ text, BarcodeType type) : m_format(format), m_text(text), m_barcode_type(type) {}

private:
	Platform::String^ m_format;
	Platform::String^ m_text;

	BarcodeType m_barcode_type;
};

} // ZXing
