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

#include "oned/ODMultiUPCEANReader.h"
#include "oned/ODUPCEANReader.h"
#include "oned/ODEAN13Reader.h"
#include "oned/ODEAN8Reader.h"
#include "oned/ODUPCAReader.h"
#include "oned/ODUPCEReader.h"
#include "DecodeHints.h"
#include "BarcodeFormat.h"
#include "Result.h"

#include <unordered_set>

namespace ZXing {

namespace OneD {

MultiUPCEANReader::MultiUPCEANReader(const DecodeHints& hints)
{
	auto formats = hints.possibleFormats();
	if (formats.empty()) {
		_readers.emplace_back(new EAN13Reader(hints));
		// UPC-A is covered by EAN-13
		_readers.emplace_back(new EAN8Reader(hints));
		_readers.emplace_back(new UPCEReader(hints));
	}
	else
	{
		_formats.insert(formats.begin(), formats.end());
		if (_formats.find(BarcodeFormat::EAN_13) != _formats.end()) {
			_readers.emplace_back(new EAN13Reader(hints));
		}
		else if (_formats.find(BarcodeFormat::UPC_A) != _formats.end()) {
			_readers.emplace_back(new UPCAReader(hints));
		}
		if (_formats.find(BarcodeFormat::EAN_8) != _formats.end()) {
			_readers.emplace_back(new EAN8Reader(hints));
		}
		if (_formats.find(BarcodeFormat::UPC_E) != _formats.end()) {
			_readers.emplace_back(new UPCEReader(hints));
		}
	}
}

MultiUPCEANReader::~MultiUPCEANReader()
{
}

Result
MultiUPCEANReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
{
	// Compute this location once and reuse it on multiple implementations
	auto range = UPCEANReader::FindStartGuardPattern(row);
	if (!range)
		return Result(DecodeStatus::NotFound);

	for (auto& reader : _readers) {
		Result result = reader->decodeRow(rowNumber, row, range);
		if (!result.isValid())
		{
			if (StatusIsError(result.status()))
				continue;
			else
				return result;
		}

		// Special case: a 12-digit code encoded in UPC-A is identical to a "0"
		// followed by those 12 digits encoded as EAN-13. Each will recognize such a code,
		// UPC-A as a 12-digit string and EAN-13 as a 13-digit string starting with "0".
		// Individually these are correct and their readers will both read such a code
		// and correctly call it EAN-13, or UPC-A, respectively.
		//
		// In this case, if we've been looking for both types, we'd like to call it
		// a UPC-A code. But for efficiency we only run the EAN-13 decoder to also read
		// UPC-A. So we special case it here, and convert an EAN-13 result to a UPC-A
		// result if appropriate.
		//
		// But, don't return UPC-A if UPC-A was not a requested format!
		const std::wstring& resultText = result.text();
		bool ean13MayBeUPCA = result.format() == BarcodeFormat::EAN_13 && !resultText.empty() && resultText[0] == '0';
		bool canReturnUPCA = _formats.empty() || _formats.find(BarcodeFormat::UPC_A) != _formats.end();
		if (ean13MayBeUPCA && canReturnUPCA) {
			result.setText(resultText.substr(1));
			result.setFormat(BarcodeFormat::UPC_A);
		}
		return result;
	}
	return Result(DecodeStatus::NotFound);
}

} // OneD
} // ZXing
