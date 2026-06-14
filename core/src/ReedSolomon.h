// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <optional>
#include <span>
#include <stdexcept>
#include <string>

namespace ZXing {

enum class RSField
{
	Aztec4,
	Aztec6,
	Aztec8,
	Aztec10,
	Aztec12,
	DataMatrix,
	MaxiCode,
	PDF417,
	QRCode
};

inline RSField GF2nAztec(int wordSize)
{
	switch (wordSize) {
	case 4: return RSField::Aztec4;
	case 6: return RSField::Aztec6;
	case 8: return RSField::Aztec8;
	case 10: return RSField::Aztec10;
	case 12: return RSField::Aztec12;
	default: throw std::invalid_argument("Unsupported word size " + std::to_string(wordSize));
	}
}

/**
 * @brief ReedSolomonDecode fixes errors in a message containing both data and parity codewords.
 *
 * @param message data and error-correction/parity codewords
 * @param numECC number of error-correction code words
 * @param erasures positions of known erasures in the message
 * @return optional unused error correction in the range [0, 1] if message errors could successfully be fixed (or there have not been
 * any), std::nullopt otherwise
 */
std::optional<double> ReedSolomonDecode(RSField field, std::span<int> message, int numECC, std::span<const int> erasures = {});

std::optional<double> ReedSolomonDecode(RSField field, std::span<uint8_t> message, int numECC,
										std::span<const int> erasures = {});

/**
 * @brief ReedSolomonEncode replaces the last numECC in message with error correction code words
 */
void ReedSolomonEncode(RSField field, std::span<const uint8_t> message, std::span<uint8_t> ecc);
void ReedSolomonEncode(RSField field, std::span<uint8_t> message, int numECC);

void ReedSolomonEncode(RSField field, std::span<int> message, int numECC);

} // ZXing
