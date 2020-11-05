/*
* Copyright 2020 Axel Waggershauser
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

#include "BitArray.h"
#include "Result.h"

#include <memory>

namespace ZXing::OneD {

Result RowReader::decodeSingleRow(int rowNumber, const BitArray& row) const
{
	std::unique_ptr<DecodingState> state;
	PatternRow res;
	auto li = row.begin();
	auto i  = li;
	if (*i)
		res.push_back(0);
	while ((i = row.getNextSetTo(i, !*i)) != row.end()) {
		res.push_back(static_cast<PatternRow::value_type>(i - li));
		li = i;
	}
	res.push_back(static_cast<PatternRow::value_type>(i - li));
	if (*(i-1))
		res.push_back(0);

	return decodePattern(rowNumber, res, state);
}

} // namespace ZXing::OneD
