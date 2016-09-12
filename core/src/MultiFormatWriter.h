#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

class BitMatrix;
class EncodeHints;
class EncodeStatus;
enum class BarcodeFormat;

/**
* This is a factory class which finds the appropriate Writer subclass for the BarcodeFormat
* requested and encodes the barcode with the supplied contents.
*
* @author dswitkin@google.com (Daniel Switkin)
*/
class MultiFormatWriter
{
public:
	static EncodeStatus Encode(const std::wstring& contents, BarcodeFormat format, int width, int height, const EncodeHints& hints, BitMatrix& output);
};

} // ZXing
