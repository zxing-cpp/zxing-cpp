// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#include "ReedSolomon.h"
#include "Version.h"

#include "librscpp/encode.h"
#include "librscpp/decode.h"

#include <algorithm>
#include <type_traits>
#include <stdexcept>
#include <utility>

namespace ZXing {

namespace rs = librscpp;

using GF2nI = rs::GF2n<>;

const GF2nI& GetGF2n(RSField field)
{
	static GF2nI gf2n_4_1(0x0013, 1);      // x^4 + x + 1
	static GF2nI gf2n_6_1(0x0043, 1);      // x^6 + x + 1
	static GF2nI gf2n_8_5_3_2(0x012D, 1);  // x^8 + x^5 + x^3 + x^2 + 1 : DataMatrix
	static GF2nI gf2n_8_4_3_2(0x011D, 0);  // x^8 + x^4 + x^3 + x^2 + 1 : QRCode
	static GF2nI gf2n_10_3(0x0409, 1);     // x^10 + x^3 + 1
	static GF2nI gf2n_12_6_5_3(0x1069, 1); // x^12 + x^6 + x^5 + x^3 + 1

	switch (field) {
	case RSField::Aztec4: return gf2n_4_1;
	case RSField::Aztec6: return gf2n_6_1;
	case RSField::Aztec8: return gf2n_8_5_3_2;
	case RSField::Aztec10: return gf2n_10_3;
	case RSField::Aztec12: return gf2n_12_6_5_3;
	case RSField::DataMatrix: return gf2n_8_5_3_2;
	case RSField::MaxiCode: return gf2n_6_1;
	case RSField::QRCode: return gf2n_8_4_3_2;
	default: throw std::invalid_argument("Unsupported RSField");
	}
}

const rs::GFp<>& GetGFPDF417()
{
	static rs::GFp<> gf929_3(929, 3, 1);
	return gf929_3;
}

// See ISO/IEC 15415:2024 7.5.9
static std::optional<double> UEC(std::optional<int> usedECC, int numECC)
{
	return usedECC ? std::optional(std::clamp(1.0 - static_cast<double>(*usedECC) / numECC, 0.0, 1.0)) : std::nullopt;
}

#ifdef ZXING_READERS
std::optional<double> ReedSolomonDecode(RSField field, std::span<uint8_t> codeword, int numECC, std::span<const int> erasures)
{
	return UEC(rs::decode(GetGF2n(field), codeword, numECC, erasures), numECC);
}

std::optional<double> ReedSolomonDecode(RSField field, std::span<int> codeword, int numECC, std::span<const int> erasures)
{
#if ZXING_ENABLE_PDF417
	if (field == RSField::PDF417)
		return UEC(rs::decode(GetGFPDF417(), codeword, numECC, erasures), numECC);
	else
#endif
		return UEC(rs::decode(GetGF2n(field), codeword, numECC, erasures), numECC);
}
#endif

#ifdef ZXING_WRITERS
void ReedSolomonEncode(RSField field, std::span<const uint8_t> data, std::span<uint8_t> parity)
{
	rs::encode(GetGF2n(field), data, parity);
}

void ReedSolomonEncode(RSField field, std::span<uint8_t> codeword, int numECC)
{
	rs::encode_inplace(GetGF2n(field), codeword, numECC);
}

void ReedSolomonEncode(RSField field, std::span<int> codeword, int numECC)
{
#if ZXING_ENABLE_PDF417
	if (field == RSField::PDF417)
		return rs::encode_inplace(GetGFPDF417(), codeword, numECC);
	else
#endif
		rs::encode_inplace(GetGF2n(field), codeword, numECC);
}
#endif

} // namespace ZXing
