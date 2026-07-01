/*
* Copyright 2025 wooyechan
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Pattern.h"

namespace ZXing::OneD::Code93 {

// Each character consist of 3 bars and 3 spaces and is 9 modules wide in total.
// Each bar and space is from 1 to 4 modules wide.
constexpr int CHAR_LEN = 6;
constexpr int CHAR_MODS = 9;

constexpr std::array<FixedPattern<CHAR_LEN, CHAR_MODS>, 48> CODE_PATTERNS = { {
	{ 1, 3, 1, 1, 1, 2 }, // 0
	{ 1, 1, 1, 2, 1, 3 },
	{ 1, 1, 1, 3, 1, 2 },
	{ 1, 1, 1, 4, 1, 1 },
	{ 1, 2, 1, 1, 1, 3 },
	{ 1, 2, 1, 2, 1, 2 }, // 5
	{ 1, 2, 1, 3, 1, 1 },
	{ 1, 1, 1, 1, 1, 4 },
	{ 1, 3, 1, 2, 1, 1 },
	{ 1, 4, 1, 1, 1, 1 },
	{ 2, 1, 1, 1, 1, 3 }, // 10
	{ 2, 1, 1, 2, 1, 2 }, 
	{ 2, 1, 1, 3, 1, 1 },
	{ 2, 2, 1, 1, 1, 2 },
	{ 2, 2, 1, 2, 1, 1 },
	{ 2, 3, 1, 1, 1, 1 }, // 15
	{ 1, 1, 2, 1, 1, 3 },
	{ 1, 1, 2, 2, 1, 2 },
	{ 1, 1, 2, 3, 1, 1 },
	{ 1, 2, 2, 1, 1, 2 },
	{ 1, 3, 2, 1, 1, 1 }, // 20
	{ 1, 1, 1, 1, 2, 3 },
	{ 1, 1, 1, 2, 2, 2 },
	{ 1, 1, 1, 3, 2, 1 },
	{ 1, 2, 1, 1, 2, 2 },
	{ 1, 3, 1, 1, 2, 1 }, // 25
	{ 2, 1, 2, 1, 1, 2 },
	{ 2, 1, 2, 2, 1, 1 },
	{ 2, 1, 1, 1, 2, 2 },
	{ 2, 1, 1, 2, 2, 1 },
	{ 2, 2, 1, 1, 2, 1 }, // 30
	{ 2, 2, 2, 1, 1, 1 },
	{ 1, 1, 2, 1, 2, 2 },
	{ 1, 1, 2, 2, 2, 1 },
	{ 1, 2, 2, 1, 2, 1 },
	{ 1, 2, 3, 1, 1, 1 }, // 35
	{ 1, 2, 1, 1, 3, 1 },
	{ 3, 1, 1, 1, 1, 2 },
	{ 3, 1, 1, 2, 1, 1 },
	{ 3, 2, 1, 1, 1, 1 },
	{ 1, 1, 2, 1, 3, 1 }, // 40
	{ 1, 1, 3, 1, 2, 1 },
	{ 2, 1, 1, 1, 3, 1 },
	{ 1, 2, 1, 2, 2, 1 },
	{ 3, 1, 2, 1, 1, 1 },
	{ 3, 1, 1, 1, 2, 1 }, // 45
	{ 1, 2, 2, 2, 1, 1 },
	{ 1, 1, 1, 1, 4, 1 }  // STOP_CODE / Asterisk
} };

constexpr auto E2E_PATTERNS = PatternsToE2EInts(CODE_PATTERNS);


// Note that 'abcd' are dummy characters in place of control characters.
// Control chars ($)==a, (%)==b, (/)==c, (+)==d
constexpr char ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";

static_assert(Size(ALPHABET) == Size(CODE_PATTERNS), "table size mismatch");

constexpr int ASTERISK_ENCODING = E2E_PATTERNS[47];

} // namespace ZXing::OneD::Code93
