/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <ZXing/BarcodeFormat.h>
#include <ZXing/ReadBarcode.h>
#include <ZXing/WriteBarcode.h>

namespace ZXing {

const std::string& Version();

#ifdef ZXING_EXPERIMENTAL_API

enum class Operation
{
	Create,
	Read,
	CreateAndRead,
	CreateOrRead,
};

BarcodeFormats SupportedBarcodeFormats(Operation op = Operation::CreateOrRead);

#endif // ZXING_EXPERIMENTAL_API

} // namespace ZXing
