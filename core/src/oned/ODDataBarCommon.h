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

#include <array>

namespace ZXing {
namespace OneD {

inline bool IsGuard(int a, int b)
{
	//	printf(" (%d, %d)", a, b);
	return a > b * 3 / 4 - 2 && a < b * 5 / 4 + 2;
}

inline bool IsCharacter(PatternView view, int modules, float modSizeRef)
{
	float err = std::abs(float(view.sum()) / modules / modSizeRef - 1);
	//	printf(" %.3f", err);
	return err < 0.1f;
}

inline bool IsCharacterPair(PatternView v, int modsFront, int modsBack)
{
	float modSizeRef = v.subView(8, 5).sum() / 15.f;
	return IsCharacter(v.subView(0, 8), modsFront, modSizeRef) && IsCharacter(v.subView(13, 8), modsBack, modSizeRef);
}

struct Character
{
	int value = -1, checksum = -1;
	operator bool() const { return value != -1; }
};

struct Pair : Character
{
	int finder = -1, xStart = -1, xStop = 1, rowNumber = -1;
};

inline bool operator==(const Pair& l, const Pair& r)
{
	return l.value == r.value && l.checksum == r.checksum && l.finder == r.finder;
}

struct PairHash
{
	std::size_t operator()(const Pair& p) const noexcept { return p.value ^ p.checksum ^ p.finder; }
};

using Array4I = std::array<int, 4>;
using Array4F = std::array<float, 4>;

bool ReadDataCharacterRaw(const PatternView& view, int numModules, bool reversed, Array4I& oddPattern,
						  Array4I& evnPattern);

} // OneD
} // ZXing
