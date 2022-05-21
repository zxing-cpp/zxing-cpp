/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

enum class DecodeStatus;

namespace OneD::DataBar {

DecodeStatus ParseFieldsInGeneralPurpose(const std::string &rawInfo, std::string& result);

} // namespace OneD::DataBar
} // namespace ZXing
