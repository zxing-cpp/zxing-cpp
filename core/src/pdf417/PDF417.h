/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BitMatrixCursor.h"
#include "Pattern.h"
#include "Point.h"
#include "ZXAlgorithms.h"

#ifndef PRINT_DEBUG
#define printf(...){}
#define printv(...){}
#else
#define printv(prefix, fmt, postfix, ...) \
printf("%s", prefix); \
for (auto v : __VA_ARGS__) \
	printf(fmt, v); \
printf("%s", postfix);
#endif

namespace ZXing::PDF417 {

constexpr int MS_THR = 2; // tolerance of pattern size in modules

using Pattern417 = std::array<uint16_t, 8>;

struct Codeword
{
	int codeword : 12 = -1;
	int cluster  : 4 = -1;
	int count    : 16 = 0;
	PointT<float> left, right;

	constexpr operator bool() const noexcept { return codeword != -1 && cluster % 3 == 0; }
	constexpr bool operator==(const Codeword& other) const noexcept { return codeword == other.codeword; }
	PointF leftPos() const noexcept { return PointF(left / count); }
	PointF rightPos() const noexcept { return PointF(right / count); }
};

struct CodewordPattern
{
	int codeword  : 11 = 0;    // 11 is enough for [-1..929)
	unsigned bits : 7 * 3 = 0; // 6/7 slots of 3 bits each for the normalized e2e pattern

	constexpr bool operator<(const CodewordPattern& other) const noexcept { return bits < other.bits; }
	constexpr int pattern(int i) const noexcept { return (bits >> ((6 - i - 1) * 3)) & 0b111; }
	constexpr int cluster() const noexcept { return (pattern(0) - pattern(1) + pattern(4) - pattern(5) + 9) % 9; }
};

constexpr int CodewordCluster(const std::array<int, 8>& np)
{
	return (np[0] - np[2] + np[4] - np[6] + 9) % 9;
}

constexpr int CodewordCluster(const std::array<int, 6>& np)
{
	return (np[0] - np[1] + np[4] - np[5] + 9) % 9;
}

template <typename POINT>
class BitMatrixModuleCursor : public BitMatrixCursor<POINT>
{
public:
	float ms;

	BitMatrixModuleCursor(const BitMatrix& image, POINT p, POINT d, float ms) : BitMatrixCursor<POINT>(image, p, d), ms(ms) {}

	BitMatrixModuleCursor movedBy(POINT o) const noexcept { return {*this->img, this->p + o, this->d, ms}; }
};

using BitMatrixModuleCursorF = BitMatrixModuleCursor<PointF>;

Codeword ReadCodeword(BitMatrixModuleCursorF& cur);
Codeword ReadCodeword(BitMatrixModuleCursorF& cur, int expectedCluster);

bool SkipCodeword(BitMatrixModuleCursorF& cur);

} // namespace ZXing::PDF417
