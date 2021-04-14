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

#include "CharacterSet.h"

#include <string>

namespace ZXing {

/**
* Encapsulates a Character Set ECI, according to AIM ITS/04-023:2004 Extended Channel Interpretations Part 3: Register
*
* @author Sean Owen
*/
namespace CharacterSetECI {

/**
 * @param value character set ECI value
 * @return {@code CharacterSet} representing ECI of given value, or {@code CharacterSet::Unknown} if it is unsupported
 */
CharacterSet CharsetFromValue(int value);

/**
 * @param charset {@code CharacterSet} representing ECI
 * @return ECI of given {@code CharacterSet}, or -1 if it is unsupported
 */
int ValueForCharset(CharacterSet charset);

/**
 * @param name character set ECI encoding name
 * @return {@code CharacterSet} representing ECI of given value, or {@code CharacterSet::Unknown} if it is
 *   unsupported
 */
CharacterSet CharsetFromName(const char* name);

/**
 * @param name character set ECI encoding name
 * @param encodingDefault default {@code CharacterSet} encoding to return if name is empty or unsupported
 * @return {@code CharacterSet} representing ECI for character encoding
 */
CharacterSet InitEncoding(const std::string& name, CharacterSet encodingDefault = CharacterSet::ISO8859_1);

/**
 * @param eci ECI requested, which if different when resolved to the existing {@code CharacterSet} encoding will
 *   cause conversion to occur
 * @param encoded Unicode buffer to append the data to if conversion occurs
 * @param data buffer to be converted and appended to Unicode buffer and then cleared if conversion occurs
 * @param encoding existing {@code CharacterSet} encoding to use for conversion if conversion occurs
 * @return {@code CharacterSet} representing ECI for new character encoding if changed, or existing encoding if not
 */
CharacterSet OnChangeAppendReset(const int eci, std::wstring& encoded, std::string& data, CharacterSet encoding);

} // namespace CharacterSetECI
} // namespace ZXing
