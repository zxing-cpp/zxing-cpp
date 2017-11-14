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

#include "pdf417/PDFWriter.h"
#include "pdf417/PDFEncoder.h"
#include "BitMatrix.h"
#include "CharacterSetECI.h"

namespace ZXing {
namespace Pdf417 {

/**
* default white space (margin) around the code
*/
static const int WHITE_SPACE = 30;

/**
* default error correction level
*/
static const int DEFAULT_ERROR_CORRECTION_LEVEL = 2;

/**
* Takes and rotates the it 90 degrees
*/
static void RotateArray(const std::vector<std::vector<bool>>& input, std::vector<std::vector<bool>>& output)
{
	size_t height = input.size();
	size_t width = input[0].size();
	output.resize(width);
	for (size_t i = 0; i < width; ++i) {
		output[i].resize(height);
	}
	for (size_t ii = 0; ii < height; ++ii) {
		// This makes the direction consistent on screen when rotating the screen
		size_t inverseii = height - ii - 1;
		for (size_t jj = 0; jj < width; ++jj) {
			output[jj][inverseii] = input[ii][jj];
		}
	}
}

/**
* This takes an array holding the values of the PDF 417
*
* @param input a byte array of information with 0 is black, and 1 is white
* @param margin border around the barcode
* @return BitMatrix of the input
*/
static BitMatrix BitMatrixFromBitArray(const std::vector<std::vector<bool>>& input, int margin)
{
	// Creates the bitmatrix with extra space for whitespace
	int width = static_cast<int>(input[0].size());
	int height = static_cast<int>(input.size());
	BitMatrix result(width + 2 * margin, height + 2 * margin);
	for (int y = 0, yOutput = static_cast<int>(result.height()) - margin - 1; y < height; y++, yOutput--) {
		for (int x = 0; x < width; ++x) {
			// Zero is white in the bytematrix
			if (input[y][x]) {
				result.set(x + margin, yOutput);
			}
		}
	}
	return result;
}

BitMatrix
Writer::encode(const std::wstring& contents, int width, int height) const
{
	int margin = _margin >= 0 ? _margin : WHITE_SPACE;
	int ecLevel = _ecLevel >= 0 ? _ecLevel : DEFAULT_ERROR_CORRECTION_LEVEL;

	BarcodeMatrix resultMatrix = _encoder->generateBarcodeLogic(contents, ecLevel);
	int aspectRatio = 4;
	std::vector<std::vector<bool>> originalScale;
	resultMatrix.getScaledMatrix(1, aspectRatio, originalScale);
	bool rotated = false;
	if ((height > width) != (originalScale[0].size() < originalScale.size())) {
		std::vector<std::vector<bool>> temp;
		RotateArray(originalScale, temp);
		originalScale = temp;
		rotated = true;
	}

	int scaleX = width / static_cast<int>(originalScale[0].size());
	int scaleY = height / static_cast<int>(originalScale.size());

	int scale;
	if (scaleX < scaleY) {
		scale = scaleX;
	}
	else {
		scale = scaleY;
	}

	if (scale > 1) {
		std::vector<std::vector<bool>> scaledMatrix;
		resultMatrix.getScaledMatrix(scale, scale * aspectRatio, scaledMatrix);
		if (rotated) {
			std::vector<std::vector<bool>> temp;
			RotateArray(scaledMatrix, temp);
			scaledMatrix = temp;
		}
		return BitMatrixFromBitArray(scaledMatrix, margin);
	}
	else {
		return BitMatrixFromBitArray(originalScale, margin);
	}
}

Writer::Writer()
{
	_encoder.reset(new Encoder);
}

Writer::Writer(Writer &&other) :
	_margin(other._margin),
	_ecLevel(other._ecLevel),
	_encoder(std::move(other._encoder))
{
}

Writer::~Writer()
{
}

Writer&
Writer::setDimensions(int minCols, int maxCols, int minRows, int maxRows)
{
	_encoder->setDimensions(minCols, maxCols, minRows, maxRows);
	return *this;
}

Writer&
Writer::setCompaction(Compaction compaction)
{
	_encoder->setCompaction(compaction);
	return *this;
}

/**
* @param compact if true, enables compaction
*/
Writer&
Writer::setCompact(bool compact)
{
	_encoder->setCompact(compact);
	return *this;
}

/**
* @param encoding sets character encoding to use
*/
Writer&
Writer::setEncoding(CharacterSet encoding)
{
	_encoder->setEncoding(encoding);
	return *this;
}


} // Pdf417
} // ZXing
