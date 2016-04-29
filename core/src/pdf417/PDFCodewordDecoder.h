#pragma once
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

#include <array>

namespace ZXing {
namespace Pdf417 {

/**
* @author Guenther Grau
* @author creatale GmbH (christoph.schulz@creatale.de)
*/
class CodewordDecoder
{
public:
	static int GetDecodedValue(const std::array<int, 8>& moduleBitCount);
};

} // Pdf417
} // ZXing
