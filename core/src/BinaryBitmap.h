#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2021 Axel Waggershauser
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

#include "ReadBarcode.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace ZXing {

class BitMatrix;

using PatternRow = std::vector<uint16_t>;

/**
* This class is the core bitmap class used by ZXing to represent 1 bit data. Reader objects
* accept a BinaryBitmap and attempt to decode it.
*/
class BinaryBitmap
{
	struct Cache;
	std::unique_ptr<Cache> _cache;

protected:
	const ImageView _buffer;

	/**
	* Converts a 2D array of luminance data to 1 bit (true means black).
	*
	* @return The 2D array of bits for the image, nullptr on error.
	*/
	virtual std::shared_ptr<const BitMatrix> getBlackMatrix() const = 0;

public:
	BinaryBitmap(const ImageView& buffer);
	virtual ~BinaryBitmap();

	int width() const { return _buffer.width(); }
	int height() const { return _buffer.height(); }

	/**
	* Converts one row of luminance data to a vector of ints denoting the widths of the bars and spaces.
	*/
	virtual bool getPatternRow(int row, int rotation, PatternRow& res) const = 0;

	const BitMatrix* getBitMatrix() const;
};

} // ZXing
