/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ImageView.h"

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
	bool _inverted = false;
	bool _closed = false;

protected:
	const ImageView _buffer;

	/**
	* Converts a 2D array of luminance data to 1 bit (true means black).
	*
	* @return The 2D array of bits for the image, nullptr on error.
	*/
	virtual std::shared_ptr<const BitMatrix> getBlackMatrix() const = 0;

	BitMatrix binarize(uint8_t threshold) const;

public:
	BinaryBitmap(const ImageView& buffer);
	virtual ~BinaryBitmap();

	int width() const { return _buffer.width(); }
	int height() const { return _buffer.height(); }
	const ImageView& imageView() const { return _buffer; }

	/**
	* Converts one row of luminance data to a vector of ints denoting the widths of the bars and spaces.
	*/
	virtual bool getPatternRow(int row, int rotation, PatternRow& res) const = 0;

	const BitMatrix* getBitMatrix(bool transposed = false) const;

	/**
	* Access to the underlying luminance for detectors that re-threshold locally
	* (per symbol). Returns -1 when the buffer is not single-channel luminance.
	*/
	int luma(int x, int y) const
	{
		return _buffer.format() == ImageFormat::Lum && x >= 0 && x < width() && y >= 0 && y < height()
				   ? *_buffer.data(x, y)
				   : -1;
	}

	void invert();
	bool inverted() const { return _inverted; }

	void close();
	bool closed() const { return _closed; }
};

} // ZXing
