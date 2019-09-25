#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
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
#include "AZToken.h"
#include <vector>

namespace ZXing {
namespace Aztec {

class Token;

/**
* State represents all information about a sequence necessary to generate the current output.
* Note that a state is immutable.
*/
class EncodingState
{
public:
	// The list of tokens that we output.  If we are in Binary Shift mode, this
	// token list does *not* yet included the token for those bytes
	std::vector<Token> tokens;

	// The current mode of the encoding (or the mode to which we'll return if
	// we're in Binary Shift mode.
	int mode;
	
	// If non-zero, the number of most recent bytes that should be output
	// in Binary Shift mode.
	int binaryShiftByteCount;
	
	// The total number of bits generated (including Binary Shift).
	int bitCount;
};

} // Aztec
} // ZXing
