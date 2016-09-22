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
#include "EncodeHints.h"
#include "EncodeStatus.h"
#include "BitMatrix.h"

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
static byte[][] rotateArray(byte[][] bitarray) {
	byte[][] temp = new byte[bitarray[0].length][bitarray.length];
	for (int ii = 0; ii < bitarray.length; ii++) {
		// This makes the direction consistent on screen when rotating the
		// screen;
		int inverseii = bitarray.length - ii - 1;
		for (int jj = 0; jj < bitarray[0].length; jj++) {
			temp[jj][inverseii] = bitarray[ii][jj];
		}
	}
	return temp;
}

/**
* This takes an array holding the values of the PDF 417
*
* @param input a byte array of information with 0 is black, and 1 is white
* @param margin border around the barcode
* @return BitMatrix of the input
*/
static BitMatrix bitMatrixFrombitArray(byte[][] input, int margin) {
	// Creates the bitmatrix with extra space for whitespace
	BitMatrix output = new BitMatrix(input[0].length + 2 * margin, input.length + 2 * margin);
	output.clear();
	for (int y = 0, yOutput = output.getHeight() - margin - 1; y < input.length; y++, yOutput--) {
		for (int x = 0; x < input[0].length; x++) {
			// Zero is white in the bytematrix
			if (input[y][x] == 1) {
				output.set(x + margin, yOutput);
			}
		}
	}
	return output;
}

/**
* Takes encoder, accounts for width/height, and retrieves bit matrix
*/
static EncodeStatus DoEncode(PDF417 encoder, String contents, int errorCorrectionLevel, int width, int height, int margin, BitMatrix& output)
{
	encoder.generateBarcodeLogic(contents, errorCorrectionLevel);

	int aspectRatio = 4;
	byte[][] originalScale = encoder.getBarcodeMatrix().getScaledMatrix(1, aspectRatio);
	boolean rotated = false;
	if ((height > width) ^ (originalScale[0].length < originalScale.length)) {
		originalScale = rotateArray(originalScale);
		rotated = true;
	}

	int scaleX = width / originalScale[0].length;
	int scaleY = height / originalScale.length;

	int scale;
	if (scaleX < scaleY) {
		scale = scaleX;
	}
	else {
		scale = scaleY;
	}

	if (scale > 1) {
		byte[][] scaledMatrix =
			encoder.getBarcodeMatrix().getScaledMatrix(scale, scale * aspectRatio);
		if (rotated) {
			scaledMatrix = rotateArray(scaledMatrix);
		}
		return bitMatrixFrombitArray(scaledMatrix, margin);
	}
	return bitMatrixFrombitArray(originalScale, margin);
}

EncodeStatus
Writer::Encode(const std::wstring& contents, int width, int height, const EncodeHints& hints, BitMatrix& output)
{
	PDF417 encoder = new PDF417();
	int margin = WHITE_SPACE;
	int errorCorrectionLevel = DEFAULT_ERROR_CORRECTION_LEVEL;

	if (hints != null) {
		if (hints.containsKey(EncodeHintType.PDF417_COMPACT)) {
			encoder.setCompact(Boolean.valueOf(hints.get(EncodeHintType.PDF417_COMPACT).toString()));
		}
		if (hints.containsKey(EncodeHintType.PDF417_COMPACTION)) {
			encoder.setCompaction(Compaction.valueOf(hints.get(EncodeHintType.PDF417_COMPACTION).toString()));
		}
		if (hints.containsKey(EncodeHintType.PDF417_DIMENSIONS)) {
			Dimensions dimensions = (Dimensions)hints.get(EncodeHintType.PDF417_DIMENSIONS);
			encoder.setDimensions(dimensions.getMaxCols(),
				dimensions.getMinCols(),
				dimensions.getMaxRows(),
				dimensions.getMinRows());
		}
		if (hints.containsKey(EncodeHintType.MARGIN)) {
			margin = Integer.parseInt(hints.get(EncodeHintType.MARGIN).toString());
		}
		if (hints.containsKey(EncodeHintType.ERROR_CORRECTION)) {
			errorCorrectionLevel = Integer.parseInt(hints.get(EncodeHintType.ERROR_CORRECTION).toString());
		}
		if (hints.containsKey(EncodeHintType.CHARACTER_SET)) {
			Charset encoding = Charset.forName(hints.get(EncodeHintType.CHARACTER_SET).toString());
			encoder.setEncoding(encoding);
		}
	}

	return DoEncode(encoder, contents, errorCorrectionLevel, width, height, margin);
}


} // Pdf417
} // ZXing
