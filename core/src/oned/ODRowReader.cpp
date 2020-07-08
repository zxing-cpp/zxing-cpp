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

#include "ODRowReader.h"
#include "Result.h"
#include "BitArray.h"

#include <cmath>
#include <limits>
#include <numeric>
#include <memory>

namespace ZXing {
namespace OneD {

Result
RowReader::decodeSingleRow(int rowNumber, const BitArray& row) const
{
	std::unique_ptr<DecodingState> state;
	return decodeRow(rowNumber, row, state);
}

Result RowReader::decodePattern(int, const PatternView&, std::unique_ptr<RowReader::DecodingState>&) const
{
#ifdef ZX_USE_NEW_ROW_READERS
	return Result(DecodeStatus::_internal);
#else
	return Result(DecodeStatus::NotFound);
#endif
}

} // OneD
} // ZXing
