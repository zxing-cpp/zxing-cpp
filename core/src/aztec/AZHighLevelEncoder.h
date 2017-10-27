#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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

namespace Aztec {

/**
* This produces nearly optimal encodings of text into the first-level of
* encoding used by Aztec code.
*
* It uses a dynamic algorithm.  For each prefix of the string, it determines
* a set of encodings that could lead to this prefix.  We repeatedly add a
* character and generate a new set of optimal encodings until we have read
* through the entire input.
*
* @author Frank Yellin
* @author Rustam Abdullaev
*/
class HighLevelEncoder
{
public:
	static BitArray Encode(const std::string& text);
};

} // Aztec
} // ZXing
