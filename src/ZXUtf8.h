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

#include <cstdint>

namespace ZXing {

/// <summary>
/// This class offers optimized (i.e. fast) encoder and decoder for UTF-8 string.
/// </summary>
class Utf8
{
public:
	enum DecodeState
	{
		kAccepted = 0,
		kRejected = 1,
	};

	/// <summary>
	/// Count the number of code points given a UTF-8 string.
	/// </summary>
	static int CountCodePoints(const char* utf8);

	/// <summary>
	/// Count the number of code points given a UTF-8 string.
	/// </summary>
	static const char* SkipCodePoints(const char* utf8, int count);

	/// <summary>
	/// This method implements a single step in this process. It takes two parameters maintaining state and a byte,
	/// and returns the state achieved after processing the byte.Specifically, it returns the value UTF8_ACCEPT(0)
	/// if enough bytes have been read for a character, UTF8_REJECT(1) if the byte is not allowed to occur at its
	/// position, and some other positive value if more bytes have to be read.
	///
	/// When decoding the first byte of a string, the caller must set the state variable to UTF8_ACCEPT. If, after
	/// decoding one or more bytes the state UTF8_ACCEPT is reached again, then the decoded Unicode character value
	/// is available through the codep parameter.If the state UTF8_REJECT is entered, that state will never be exited
	/// unless the caller intervenes.See the examples below for more information on usage and error handling, and the
	/// section on implementation details for how the decoder is constructed.
	/// </summary>
	static uint32_t Decode(uint8_t byte, uint32_t& state, uint32_t& codep);

	/// <summary>
	/// Encode given code point in UTF-32 into a sequence of bytes which will be written to 'out'.
	/// Return the number of bytes written.
	/// Caller is responsible to make sure that there is enough room to write output bytes.
	/// A maximum of 4 bytes will be written to 'out'.
	/// </summary>
	static int Encode(uint32_t utf32, char* utf8);

	/// <summary>
	/// Count the number of bytes required to store given code points in UTF-8.
	/// </summary>
	static int CountBytes(const uint32_t* utf32, size_t count);
};

} // ZXing
