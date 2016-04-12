#pragma once
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
	/// Return the new value of state:
	/// kAccepted if enough bytes have been read for a character;
    /// kRejected if the byte is not allowed to occur at its position, and some other positive value if more bytes have to be read.
	/// </summary>
	static uint32_t Decode(uint8_t byte, uint32_t& state, uint32_t& codep);

	/// <summary>
	/// Encode given code point in UTF-32 into a sequence of bytes which will be written to 'out'.
	/// Return the number of bytes written.
	/// Caller is responsible to make sure that there is enough room to write output bytes.
	/// A maximum of 6 bytes will be written to 'out'.
	/// </summary>
	static int Encode(uint32_t utf32, char* utf8);
};

} // ZXing