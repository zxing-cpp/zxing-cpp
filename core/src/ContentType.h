/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

/**
 * @brief Provides a hint to the type of content encoded in a barcode, such as text, binary data, etc.
 */
enum class ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };

std::string ToString(ContentType type);

} // ZXing
