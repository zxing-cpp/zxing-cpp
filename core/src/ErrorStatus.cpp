#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "ErrorStatus.h"
#include "BitHacks.h"

namespace ZXing {

bool
StatusIsKindOf(ErrorStatus status, ErrorStatus group)
{
	int shift = BitHacks::NumberOfTrailingZeros(static_cast<uint32_t>(group));
	return (static_cast<uint32_t>(status) >> shift) == (static_cast<uint32_t>(group) >> shift);
}

} // ZXing
