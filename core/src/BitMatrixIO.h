#pragma once
/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2017 Axel Waggershauser
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

#include <string>

#include "BitMatrix.h"

namespace ZXing {
    
	std::string ToString(const BitMatrix& matrix, char one = 'X', char zero = ' ', bool addSpace = true, bool printAsCString = false);
	BitMatrix ParseBitMatrix(const std::string& str, char one = 'X', bool expectSpace = true);
	void SaveAsPBM(const BitMatrix& matrix, const std::string filename, int quiteZone = 0);

} // ZXing
