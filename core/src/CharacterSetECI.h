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

namespace ZXing {

enum class CharacterSet;

/**
* Encapsulates a Character Set ECI, according to "Extended Channel Interpretations" 5.3.1.1
* of ISO 18004.
*
* @author Sean Owen
*/
class CharacterSetECI
{
public:
	/**
	* @param value character set ECI value
	* @return {@code CharacterSetECI} representing ECI of given value, or null if it is legal but
	*   unsupported
	* @throws FormatException if ECI value is invalid
	*/
	static CharacterSet CharsetFromValue(int value);

	static int ValueForCharset(CharacterSet charset);

	/**
	* @param name character set ECI encoding name
	* @return CharacterSetECI representing ECI for character encoding, or null if it is legal
	*   but unsupported
	*/
	static CharacterSet CharsetFromName(const char* name);
};

} // ZXing
