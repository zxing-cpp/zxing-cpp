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
* @author SITA Lab (kevin.osullivan@sita.aero)
* @author Guenther Grau
*/
class Common
{
public:
	static const int NUMBER_OF_CODEWORDS = 929;
	// Maximum Codewords (Data + Error).
	static const int MAX_CODEWORDS_IN_BARCODE = NUMBER_OF_CODEWORDS - 1;
	static const int MIN_ROWS_IN_BARCODE = 3;
	static const int MAX_ROWS_IN_BARCODE = 90;
	// One left row indication column + max 30 data columns + one right row indicator column
	//public static final int MAX_CODEWORDS_IN_ROW = 32;
	static const int MODULES_IN_CODEWORD = 17;
	static const int MODULES_IN_STOP_PATTERN = 18;
	static const int BARS_IN_MODULE = 8;

	static const int SYMBOL_COUNT = 2787;

	/**
	* The sorted table of all possible symbols. Extracted from the PDF417
	* specification. The index of a symbol in this table corresponds to the
	* index into the codeword table.
	*/
	static const std::array<int, SYMBOL_COUNT> SYMBOL_TABLE;

	/**
	* This table contains to codewords for all symbols.
	*/
	static const std::array<int, SYMBOL_COUNT> CODEWORD_TABLE;

	/**
	* @param symbol encoded symbol to translate to a codeword
	* @return the codeword corresponding to the symbol.
	*/
	static int GetCodeword(int symbol);
};

} // Pdf417
} // ZXing
