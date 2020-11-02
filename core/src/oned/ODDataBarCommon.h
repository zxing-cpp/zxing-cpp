#pragma once
/*
* Copyright 2020 Axel Waggershauser
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Pattern.h"
#include "ODRowReader.h"
#include "Result.h"

#include <array>

namespace ZXing::OneD::DataBar {

inline bool IsFinder(int a, int b, int c, int d, int e)
{
	//  a,b,c,d,e, g | sum(a..e) = 15
	//  ------------
	//  1,1,2
	//	| | |,1,1, 1
	//	3,8,9

	// use only pairs of bar+space to limit effect of poor threshold:
	// b+c can be 10, 11 or 12 modules, d+e is always 2
	int w = 2 * (b + c), n = d + e;
	// the offsets (5 and 2) are there to reduce quantization effects for small module sizes
	// TODO: review after switch to sub-pixel bar width calculation
	bool x = (w + 5 > 9 * n) &&
			 (w - 5 < 13 * n) &&
//			 (b < 5 + 9 * d) &&
//			 (c < 5 + 10 * e) &&
			 (a < 2 + 4 * e) &&
			 (4 * a > n);
#if !defined(NDEBUG) && 0
	printf("[");
	for (bool v :
		 {w + 5 > 9 * n,
		  w - 5 < 13 * n,
//		  b < 5 + 9 * d,
//		  c < 5 + 10 * e,
		  a < 2 + 4 * e,
			4 * a > n})
		printf(" %d", v);
	printf("]"); fflush(stdout);
#endif
	return x;
};

inline PatternView Finder(const PatternView& view)
{
	return view.subView(8, 5);
}

inline PatternView LeftChar(const PatternView& view)
{
	return view.subView(0, 8);
}

inline PatternView RightChar(const PatternView& view)
{
	return view.subView(13, 8);
}

inline float ModSizeFinder(const PatternView& view)
{
	return Finder(view).sum() / 15.f;
}

inline bool IsGuard(int a, int b)
{
	//	printf(" (%d, %d)", a, b);
	return a > b * 3 / 4 - 2 && a < b * 5 / 4 + 2;
}

inline bool IsCharacter(const PatternView& view, int modules, float modSizeRef)
{
	float err = std::abs(float(view.sum()) / modules / modSizeRef - 1);
	return err < 0.1f;
}

struct Character
{
	int value = -1, checksum = 0;

	operator bool() const noexcept { return value != -1; }
	bool operator==(const Character& o) const noexcept { return value == o.value && checksum == o.checksum; }
	bool operator!=(const Character& o) const { return !(*this == o); }
};

struct Pair
{
	Character left, right;
	int finder = 0, xStart = -1, xStop = 1, y = -1, count = 1;

	operator bool() const noexcept { return finder != 0; }
	bool operator==(const Pair& o) const noexcept { return finder == o.finder && left == o.left && right == o.right; }
	bool operator!=(const Pair& o) const noexcept { return !(*this == o); }
};

struct PairHash
{
	std::size_t operator()(const Pair& p) const noexcept
	{
		return p.left.value ^ p.left.checksum ^ p.right.value ^ p.right.checksum ^ p.finder;
	}
};

constexpr int FULL_PAIR_SIZE = 8 + 5 + 8;
constexpr int HALF_PAIR_SIZE = 8 + 5 + 2; // half has to be followed by a guard pattern

template<typename T>
int ParseFinderPattern(const PatternView& view, bool reversed, T l2rPattern, T r2lPattern)
{
	constexpr float MAX_AVG_VARIANCE        = 0.2f;
	constexpr float MAX_INDIVIDUAL_VARIANCE = 0.45f;

	int i = 1 + RowReader::DecodeDigit(view, reversed ? r2lPattern : l2rPattern, MAX_AVG_VARIANCE,
									   MAX_INDIVIDUAL_VARIANCE, true);
	return reversed ? -i : i;
}

using Array4I = std::array<int, 4>;

bool ReadDataCharacterRaw(const PatternView& view, int numModules, bool reversed, Array4I& oddPattern,
						  Array4I& evnPattern);

Position EstimatePosition(const Pair& first, const Pair& last);

} // namespace ZXing::OneD::DataBar
