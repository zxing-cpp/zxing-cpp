/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ReadBarcode.h"
#include "WriteBarcode.h"

#ifdef ZXING_EXPERIMENTAL_API

namespace ZXing {

enum class Operation
{
	Create,
	Read,
	CreateAndRead,
	CreateOrRead,
};

BarcodeFormats SupportedBarcodeFormats(Operation op = Operation::CreateOrRead);

} // namespace ZXing

#endif // ZXING_EXPERIMENTAL_API
