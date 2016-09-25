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
#include "EncodeHints.h"
#include "EncodeStatus.h"
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
static void BitMatrixFromBitArray(const std::vector<std::vector<bool>>& input, int margin, BitMatrix& output)
{
	// Creates the bitmatrix with extra space for whitespace
	output.init(input[0].size() + 2 * margin, input.size() + 2 * margin);
	for (size_t y = 0, yOutput = output.height() - margin - 1; y < input.size(); y++, yOutput--) {
		for (size_t x = 0; x < input[0].size(); ++x) {
			// Zero is white in the bytematrix
			if (input[y][x]) {
				output.set(x + margin, yOutput);
			}
		}
	}
}

/**
* Takes encoder, accounts for width/height, and retrieves bit matrix
*/
static EncodeStatus DoEncode(const Encoder& encoder, const std::wstring contents, int errorCorrectionLevel, int width, int height, int margin, BitMatrix& output)
{
	BarcodeMatrix resultMatrix;
	EncodeStatus status = encoder.generateBarcodeLogic(contents, errorCorrectionLevel, resultMatrix);
	if (status.isOK()) {
		int aspectRatio = 4;
		std::vector<std::vector<bool>> originalScale;
		resultMatrix.getScaledMatrix(1, aspectRatio, originalScale);
		bool rotated = false;
		if ((height > width) ^ (originalScale[0].size() < originalScale.size())) {
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
			BitMatrixFromBitArray(scaledMatrix, margin, output);
		}
		else {
			BitMatrixFromBitArray(originalScale, margin, output);
		}
	}
	return status;
}

EncodeStatus
Writer::Encode(const std::wstring& contents, int width, int height, const EncodeHints& hints, BitMatrix& output)
{
	Encoder encoder;
	encoder.setCompact(hints.pdf417Compact());
	encoder.setCompaction(Compaction(hints.pdf417Compaction()));
	const int* dim = hints.pdf417Dimensions();
	if (dim[0] != 0 || dim[1] != 0 || dim[2] != 0 || dim[3] != 0) {
		encoder.setDimensions(dim[0], dim[1], dim[2], dim[3]);
	}
	int margin = hints.margin();
	if (margin == -1) {
		margin = WHITE_SPACE;
	}
	int errorCorrectionLevel = hints.errorCorrection();
	if (errorCorrectionLevel < 0) {
		errorCorrectionLevel = DEFAULT_ERROR_CORRECTION_LEVEL;
	}
	std::string charset = hints.characterSet();
	if (!charset.empty()) {
		auto encoding = CharacterSetECI::CharsetFromName(charset.c_str());
		if (encoding != CharacterSet::Unknown) {
			encoder.setEncoding(encoding);
		}
	}
	return DoEncode(encoder, contents, errorCorrectionLevel, width, height, margin, output);
}


} // Pdf417
} // ZXing
