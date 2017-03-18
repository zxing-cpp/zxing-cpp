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

#include <string>

namespace ZXing {

class BitArray;
enum class DecodeStatus;

namespace OneD {
namespace RSS {

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
* @author Eduardo Castillejo, University of Deusto (eduardo.castillejo@deusto.es)
*/
class GenericAppIdDecoder
{
public:
	static int ExtractNumeric(const BitArray& bits, int pos, int count);
	static DecodeStatus DecodeGeneralPurposeField(const BitArray& bits, int pos, std::string& result);
	static DecodeStatus DecodeAllCodes(const BitArray& bits, int initialPosition, std::string& result);
};

} // RSS
} // OneD
} // ZXing
