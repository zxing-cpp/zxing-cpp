#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include <string>

namespace ZXing {

enum class CharacterSet;

namespace QRCode {

enum class ErrorCorrectionLevel;
class EncodeResult;

/**
* @author satorux@google.com (Satoru Takabayashi) - creator
* @author dswitkin@google.com (Daniel Switkin) - ported from C++
*/
class Encoder
{
public:
	/**
	* @param content text to encode
	* @param ecLevel error correction level to use
	* @return {@link QRCode} representing the encoded QR code
	* @throws WriterException if encoding can't succeed, because of for example invalid content
	*   or configuration
	*/
	static EncodeResult Encode(const std::wstring& content, ErrorCorrectionLevel ecLevel, CharacterSet encoding, int versionNumber, bool useGs1Format);
};

} // QRCode
} // ZXing
