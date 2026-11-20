// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <span>

namespace ZXing::WXCode {

inline constexpr unsigned FORMAT_COPY_BITS = 18;

struct FormatInformation
{
	unsigned version;
	unsigned ecBits;
	unsigned mask;
	unsigned distance;
};

struct ECProfile
{
	unsigned ecCodewords;
	unsigned blockCount;
	unsigned dataCodewords;
};

// Each format copy contains 18 bits. Up to three differing bits are corrected.
bool DecodeFormat(uint32_t first, uint32_t second, FormatInformation& out);
uint32_t FormatWord(unsigned index);
std::span<const uint32_t> FormatMask();
// 72 angular rows; low 16 bits select radial cells. Storage is static.
std::span<const uint32_t> VersionDataMask(unsigned version);
unsigned VersionTotalCodewords(unsigned version);
unsigned VersionCounterCount(unsigned version);
// ecIndex is L=0, M=1, Q=2, H=3; format ecBits maps to {1,0,3,2}.
// Unsupported version/index yields nullptr.
const ECProfile* VersionEC(unsigned version, unsigned ecIndex);
unsigned MaskBit(unsigned mask, unsigned angle, unsigned radius);

} // namespace ZXing::WXCode
