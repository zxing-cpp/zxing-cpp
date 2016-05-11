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

#include "oned/ODUPCAReader.h"
#include "Result.h"

namespace ZXing {
namespace OneD {

static Result MaybeReturnResult(const Result& result)
{
	std::wstring text = result.text();
	if (!text.empty() && text[0] == '0') {
		return Result(text.substr(1), ByteArray(), result.resultPoints(), BarcodeFormat::UPC_A);
	}
	else {
		return Result(ErrorStatus::FormatError);
	}
}

Result
UPCAReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
{
	return MaybeReturnResult(_reader.decodeRow(rowNumber, row, state));
}

Result
UPCAReader::decodeRow(int rowNumber, const BitArray& row, int startGuardBegin, int startGuardEnd) const
{
	return MaybeReturnResult(_reader.decodeRow(rowNumber, row, startGuardBegin, startGuardEnd));
}

BarcodeFormat
UPCAReader::expectedFormat() const
{
	return BarcodeFormat::UPC_A;
}

ErrorStatus
UPCAReader::decodeMiddle(const BitArray& row, int &rowOffset, std::string& resultString) const
{
	return _reader.decodeMiddle(row, rowOffset, resultString);
}

} // OneD
} // ZXing
