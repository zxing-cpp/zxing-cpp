#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

#include <cstddef>
#include <cstdint>
#include <string>

namespace ZXing {

enum class CharacterSet;

class TextDecoder
{
public:
	static CharacterSet DefaultEncoding();
	static CharacterSet GuessEncoding(const uint8_t* bytes, size_t length, CharacterSet fallback = DefaultEncoding());

	static void Append(std::wstring& str, const uint8_t* bytes, size_t length, CharacterSet charset);

	static void AppendLatin1(std::wstring& str, const std::string& latin1) {
		auto ptr = (const uint8_t*)latin1.data();
		str.append(ptr, ptr + latin1.length());
	}
	
	static std::wstring FromLatin1(const std::string& latin1) {
		auto ptr = (const uint8_t*)latin1.data();
		return std::wstring(ptr, ptr + latin1.length());
	}

	static std::wstring ToUnicode(const std::string& str, CharacterSet charset) {
		std::wstring r;
		Append(r, (const uint8_t*)str.data(), str.length(), charset);
		return r;
	}

};

} // ZXing
