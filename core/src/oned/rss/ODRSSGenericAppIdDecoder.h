/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

class BitArray;
enum class DecodeStatus;

namespace OneD::DataBar {

DecodeStatus DecodeAppIdGeneralPurposeField(const BitArray& bits, int& pos, int& remainingValue, std::string& result);
DecodeStatus DecodeAppIdAllCodes(const BitArray& bits, int pos, const int remainingValue, std::string& result);

} // namespace OneD::DataBar
} // namespace ZXing
