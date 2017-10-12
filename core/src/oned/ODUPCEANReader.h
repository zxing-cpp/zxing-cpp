#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

#include "oned/ODRowReader.h"

#include <string>
#include <array>
#include <vector>

namespace ZXing {

class DecodeHints;
enum class BarcodeFormat;
enum class DecodeStatus;

namespace OneD {

/**
* <p>Encapsulates functionality and implementation that is common to UPC and EAN families
* of one-dimensional barcodes.</p>
*
* @author dswitkin@google.com (Daniel Switkin)
* @author Sean Owen
* @author alasdair@google.com (Alasdair Mackintosh)
*/
class UPCEANReader : public RowReader
{
public:
	Result decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const override;

	/**
	* <p>Like {@link #decodeRow(int, BitArray, java.util.Map)}, but
	* allows caller to inform method about where the UPC/EAN start pattern is
	* found. This allows this to be computed once and reused across many implementations.</p>
	*
	* @param rowNumber row index into the image
	* @param row encoding of the row of the barcode image
	* @param startGuardRange start/end column where the opening start pattern was found
	* @param hints optional hints that influence decoding
	* @return {@link Result} encapsulating the result of decoding a barcode in the row
	* @throws NotFoundException if no potential barcode is found
	* @throws ChecksumException if a potential barcode is found but does not pass its checksum
	* @throws FormatException if a potential barcode is found but format is invalid
	*/
	virtual Result decodeRow(int rowNumber, const BitArray& row, BitArray::Range startGuard) const;

	using Digit = std::array<int, 4>;

protected:
	explicit UPCEANReader(const DecodeHints& hints);

	/**
	* Get the format of this decoder.
	*/
	virtual BarcodeFormat expectedFormat() const = 0;

	/**
	* Subclasses override this to decode the portion of a barcode between the start
	* and end guard patterns.
	*
	* @param row row of black/white values to search
	* @param rowOffset on input, end offset of start guard pattern, and on output: horizontal offset of first pixel after the "middle" that was decoded
	* @param resultString {@link StringBuilder} to append decoded chars to
	* @throws NotFoundException if decoding could not complete successfully
	*/
	virtual DecodeStatus decodeMiddle(const BitArray& row, int &rowOffset, std::string& resultString) const = 0;

	/**
	* @param s string of digits to check
	* @return {@link #checkStandardUPCEANChecksum(CharSequence)}
	* @throws FormatException if the string does not contain only digits
	*/
	virtual	DecodeStatus checkChecksum(const std::string& s) const;


	virtual BitArray::Range decodeEnd(const BitArray& row, BitArray::Iterator begin) const;

public:
	static BitArray::Range FindStartGuardPattern(const BitArray& row);

	template <typename Container>
	static BitArray::Range FindGuardPattern(const BitArray& row, BitArray::Iterator begin, bool whiteFirst, const Container& pattern) {
		return FindGuardPattern(row, begin, whiteFirst, pattern.data(), pattern.size());
	}

	/**
	* Attempts to decode a single UPC/EAN-encoded digit.
	*
	* @param row row of black/white values to decode
	* @param counters the counts of runs of observed black/white/black/... values
	* @param rowOffset horizontal offset to start decoding from
	* @param patterns the set of patterns to use to decode -- sometimes different encodings
	* for the digits 0-9 are used, and this indicates the encodings for 0 to 9 that should
	* be used
	* @return horizontal offset of first pixel beyond the decoded digit
	* @throws NotFoundException if digit cannot be decoded
	*/
	template <size_t N>
	static DecodeStatus DecodeDigit(const BitArray& row, int rowOffset, const std::array<Digit, N>& patterns, Digit& counters, int &resultOffset) {
		return DecodeDigit(row, rowOffset, patterns.data(), patterns.size(), counters, resultOffset);
	}
	
private:
	std::vector<int> _allowedExtensions;

	static BitArray::Range FindGuardPattern(const BitArray& row, BitArray::Iterator begin, bool whiteFirst, const int* pattern, size_t length);
	static DecodeStatus DecodeDigit(const BitArray& row, int rowOffset, const Digit* patterns, size_t patternCount, Digit& counters, int &resultOffset);
};

} // OneD
} // ZXing
