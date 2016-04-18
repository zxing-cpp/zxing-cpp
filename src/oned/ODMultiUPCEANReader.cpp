/*
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
#include "DecodeHints.h"
#include "BarcodeFormat.h"

#include <unordered_set>

namespace ZXing {

namespace OneD {

MultiUPCEANReader::MultiUPCEANReader(const DecodeHints* hints)
{
	if (hints != nullptr)
	{
		auto possibleFormats = hints->getFormatList(DecodeHint::POSSIBLE_FORMATS);
		if (!possibleFormats.empty()) {
			std::unordered_set<BarcodeFormat> formats(possibleFormats.begin(), possibleFormats.end());
			if (formats.find(BarcodeFormat::EAN_13) != formats.end()) {
				_readers.push_back(std::make_shared<EAN13Reader>());
			}
			else if (formats.find(BarcodeFormat::UPC_A) != formats.end()) {
				_readers.push_back(std::make_shared<UPCAReader>());
			}
			if (formats.find(BarcodeFormat::EAN_8) != formats.end()) {
				_readers.push_back(std::make_shared<EAN8Reader>());
			}
			if (formats.find(BarcodeFormat::UPC_E) != formats.end()) {
				_readers.push_back(std::make_shared<UPCEReader>());
			}
		}
	}
	if (_readers.empty()) {
		_readers.push_back(std::make_shared<EAN13Reader>());
		// UPC-A is covered by EAN-13
		_readers.push_back(std::make_shared<EAN8Reader>());
		_readers.push_back(std::make_shared<UPCEReader>());
	}
}

Result
MultiUPCEANReader::decodeRow(int rowNumber, const BitArray& row, const DecodeHints* hints) const
{
	// Compute this location once and reuse it on multiple implementations
	int[] startGuardPattern = UPCEANReader.findStartGuardPattern(row);
	for (UPCEANReader reader : readers) {
		Result result;
		try {
			result = reader.decodeRow(rowNumber, row, startGuardPattern, hints);
		}
		catch (ReaderException ignored) {
			continue;
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
		boolean ean13MayBeUPCA =
			result.getBarcodeFormat() == BarcodeFormat.EAN_13 &&
			result.getText().charAt(0) == '0';
		@SuppressWarnings("unchecked")
		Collection<BarcodeFormat> possibleFormats =
			hints == null ? null : (Collection<BarcodeFormat>) hints.get(DecodeHintType.POSSIBLE_FORMATS);
		boolean canReturnUPCA = possibleFormats == null || possibleFormats.contains(BarcodeFormat.UPC_A);

		if (ean13MayBeUPCA && canReturnUPCA) {
			// Transfer the metdata across
			Result resultUPCA = new Result(result.getText().substring(1),
				result.getRawBytes(),
				result.getResultPoints(),
				BarcodeFormat.UPC_A);
			resultUPCA.putAllMetadata(result.getResultMetadata());
			return resultUPCA;
		}
		return result;
	}

	throw NotFoundException.getNotFoundInstance();
}

} // OneD
} // ZXing
