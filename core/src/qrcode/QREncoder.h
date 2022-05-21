/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

enum class CharacterSet;

namespace QRCode {

enum class ErrorCorrectionLevel;
class EncodeResult;

EncodeResult Encode(const std::wstring& content, ErrorCorrectionLevel ecLevel, CharacterSet encoding, int versionNumber,
					bool useGs1Format, int maskPattern = -1);

} // QRCode
} // ZXing
