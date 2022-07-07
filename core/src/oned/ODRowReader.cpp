/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

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
		res.push_back(narrow_cast<PatternRow::value_type>(i - li));
		li = i;
	}
	res.push_back(narrow_cast<PatternRow::value_type>(i - li));
	if (*(i-1))
		res.push_back(0);

	PatternView view(res);
	return decodePattern(rowNumber, view, state);
}

} // namespace ZXing::OneD
