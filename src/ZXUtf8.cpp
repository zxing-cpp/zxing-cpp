#include "ZXUtf8.h"

#ifdef _MSC_VER
	#define __builtin_prefetch(x, y, z)		// what is the equivalent with VC++?
#endif

#define ONEMASK ((unsigned)(-1) / 0xFF)

#define UTF8_ACCEPT = 0;
#define UTF8_REJECT 1;

namespace ZXing {

namespace {

	// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	const uint8_t kUtf8Data[] =
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
		0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
		0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
		0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
		1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
		1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
		1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
	};

} // anonymous

// This code ,originally named cp_strlen_utf8, is written by Colin Percival.
// See http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html for more details
int
Utf8::CountCodePoints(const char *_s)
{
	const char *s = _s;
	int count = 0;
	unsigned u;
	unsigned char b;

	/* Handle any initial misaligned bytes. */
	for (s = _s; (uintptr_t)(s) & (sizeof(unsigned) - 1); s++) {
		b = *s;

		/* Exit if we hit a zero byte. */
		if (b == '\0')
			goto done;

		/* Is this byte NOT the first byte of a character? */
		count += (b >> 7) & ((~b) >> 6);
	}

	/* Handle complete blocks. */
	for (; ; s += sizeof(unsigned)) {
		/* Prefetch 256 bytes ahead. */
		__builtin_prefetch(&s[256], 0, 0);

		/* Grab 4 or 8 bytes of UTF-8 data. */
		u = *(unsigned *)(s);

		/* Exit the loop if there are any zero bytes. */
		if ((u - ONEMASK) & (~u) & (ONEMASK * 0x80))
			break;

		/* Count bytes which are NOT the first byte of a character. */
		u = ((u & (ONEMASK * 0x80)) >> 7) & ((~u) >> 6);
		count += (u * ONEMASK) >> ((sizeof(unsigned) - 1) * 8);
	}

	/* Take care of any left-over bytes. */
	for (; ; s++) {
		b = *s;

		/* Exit if we hit a zero byte. */
		if (b == '\0')
			break;

		/* Is this byte NOT the first byte of a character? */
		count += (b >> 7) & ((~b) >> 6);
	}

done:
	return static_cast<int>((s - _s) - count);
}


/// <summary>
/// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
/// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
/// </summary>
uint32_t
Utf8::Decode(uint8_t byte, uint32_t& state, uint32_t& codep)
{
	uint32_t type = kUtf8Data[byte];
	codep = (state != kAccepted) ? (byte & 0x3fu) | (codep << 6) : (0xff >> type) & (byte);
	state = kUtf8Data[256 + state*16 + type];
	return state;
}

int
Utf8::Encode(uint32_t utf32, char* out)
{
	if (utf32 < 0x80)
	{
		*out++ = static_cast<uint8_t>(utf32);
		return 1;
	}
	if (utf32 < 0x800)
	{
		*out++ = static_cast<uint8_t>((utf32 >> 6)			| 0xc0);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f)		| 0x80);
		return 2;
	}
	if (utf32 < 0x10000)
	{
		*out++ = static_cast<uint8_t>((utf32 >> 12)			| 0xe0);
		*out++ = static_cast<uint8_t>(((utf32 >> 6) & 0x3f)	| 0x80);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f)		| 0x80);
		return 3;
	}

	*out++ = static_cast<uint8_t>((utf32 >> 18)				| 0xf0);
	*out++ = static_cast<uint8_t>(((utf32 >> 12) & 0x3f)	| 0x80);
	*out++ = static_cast<uint8_t>(((utf32 >> 6) & 0x3f)		| 0x80);
	*out++ = static_cast<uint8_t>((utf32 & 0x3f)			| 0x80);
	return 4;
}

} // ZXing