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
#include "BitMatrix.h"

#include <vector>
#include <cstddef>

namespace ZXing {
namespace OneD {

/**
* <p>Encapsulates functionality and implementation that is common to one-dimensional barcodes.</p>
*
* @author dsbnatut@gmail.com (Kazuki Nishiura)
*/
class WriterHelper
{
	static int AppendPattern(std::vector<bool>& target, int pos, const int* pattern, size_t patternCount, bool startColor);
public:
	/**
	* @return a byte array of horizontal pixels (0 = white, 1 = black)
	*/
	static BitMatrix RenderResult(const std::vector<bool>& code, int width, int height, int sidesMargin);

	/**
	* @param target encode black/white pattern into this array
	* @param pos position to start encoding at in {@code target}
	* @param pattern lengths of black/white runs to encode
	* @param startColor starting color - false for white, true for black
	* @return the number of elements added to target.
	*/
	template <typename Container>
	static int AppendPattern(std::vector<bool>& target, int pos, const Container& pattern, bool startColor) {
		return AppendPattern(target, pos, pattern.data(), pattern.size(), startColor);
	}
};

} // OneD
} // ZXing
