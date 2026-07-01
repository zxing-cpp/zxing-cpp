/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "ODWriterHelper.h"

#include "BitMatrix.h"

#include <algorithm>

namespace ZXing::OneD {

/**
* @return a byte array of horizontal pixels (0 = white, 1 = black)
*/
BitMatrix
WriterHelper::RenderResult(const std::vector<bool>& code, int width, int height, int sidesMargin)
{
	int inputWidth = Size(code);
	// Add quiet zone on both sides.
	int fullWidth = inputWidth + sidesMargin;
	int outputWidth = std::max(width, fullWidth);
	int outputHeight = std::max(1, height);

	int multiple = outputWidth / fullWidth;
	int leftPadding = (outputWidth - (inputWidth * multiple)) / 2;

	BitMatrix result(outputWidth, outputHeight);
	for (int inputX = 0, outputX = leftPadding; inputX < inputWidth; inputX++, outputX += multiple) {
		if (code[inputX]) {
			result.setRegion(outputX, 0, multiple, outputHeight);
		}
	}
	return result;
}

} // namespace ZXing::OneD
