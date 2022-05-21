/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

class DecoderResult;
class BitMatrix;

namespace DataMatrix {

/**
 * @brief Decodes a Data Matrix Code represented as a {@link BitMatrix}. A 1 or "true" is taken
 * to mean a black module.
 *
 * @param bits booleans representing white/black Data Matrix Code modules
 * @param characterSet initial character encoding to use as a {@link CharacterSetECI} name string
 * @return text and bytes encoded within the Data Matrix Code
 * @throws FormatException if the Data Matrix Code cannot be decoded
 * @throws ChecksumException if error correction fails
 */
DecoderResult Decode(const BitMatrix& bits, const std::string& characterSet = "");

} // DataMatrix
} // ZXing
