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

#include "BinaryBitmap.h"

#include "BitArray.h"
#include "Pattern.h"

#include <memory>
#include <stdexcept>

namespace ZXing {

bool BinaryBitmap::getPatternRow(int y, PatternRow& res) const
{
	res.clear();
	BitArray row;
	getBlackRow(y, row);

	auto li = row.begin();
	auto i  = li;
	if (*i)
		res.push_back(0);
	while ((i = row.getNextSetTo(i, !*i)) != row.end()) {
		res.push_back(static_cast<PatternRow::value_type>(i - li));
		li = i;
	}
	res.push_back(static_cast<PatternRow::value_type>(i - li));

	return true;
}

} // ZXing
