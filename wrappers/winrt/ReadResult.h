/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

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
