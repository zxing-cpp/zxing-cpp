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
#include "BitMatrix.h"

namespace ZXing {
namespace Aztec {

/**
* Aztec 2D code representation
*
* @author Rustam Abdullaev
*/
struct EncodeResult
{
	bool compact;
	int size;
	int layers;
	int codeWords;
	BitMatrix matrix;
};

/**
* Generates Aztec 2D barcodes.
*
* @author Rustam Abdullaev
*/
class Encoder
{
public:
	static const int DEFAULT_EC_PERCENT = 33; // default minimal percentage of error check words
	static const int DEFAULT_AZTEC_LAYERS = 0;

	static EncodeResult Encode(const std::string& data, int minECCPercent, int userSpecifiedLayers);
};

} // Aztec
} // ZXing
