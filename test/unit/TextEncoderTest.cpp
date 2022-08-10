/*
 * Copyright 2021 gitlost
 */
// SPDX-License-Identifier: Apache-2.0

#include "CharacterSet.h"
#include "TextDecoder.h"
#include "TextEncoder.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace testing;

void EnDeCode(CharacterSet cs, const char* in, std::string_view out)
{
	std::string bytes = TextEncoder::FromUnicode(in, cs);
	EXPECT_EQ(bytes, out);

	std::string dec;
	TextDecoder::Append(dec, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size(), cs);
	EXPECT_EQ(dec, in);
}

TEST(TextEncoderTest, FullCycleEncodeDecode)
{
	EnDeCode(CharacterSet::Cp437, "\u00C7", "\x80"); // LATIN CAPITAL LETTER C WITH CEDILLA
	EnDeCode(CharacterSet::ISO8859_1, "\u00A0", "\xA0"); // NO-BREAK SPACE
	EnDeCode(CharacterSet::ISO8859_2, "\u0104", "\xA1"); // LATIN CAPITAL LETTER A WITH OGONEK
	EnDeCode(CharacterSet::ISO8859_3, "\u0126", "\xA1"); // LATIN CAPITAL LETTER H WITH STROKE
	EnDeCode(CharacterSet::ISO8859_4, "\u0138", "\xA2"); // LATIN SMALL LETTER KRA
	EnDeCode(CharacterSet::ISO8859_5, "\u045F", "\xFF"); // CYRILLIC SMALL LETTER DZHE
	EnDeCode(CharacterSet::ISO8859_6, "\u0652", "\xF2"); // ARABIC SUKUN
	EnDeCode(CharacterSet::ISO8859_7, "\u03CE", "\xFE"); // GREEK SMALL LETTER OMEGA WITH TONOS
	EnDeCode(CharacterSet::ISO8859_8, "\u05EA", "\xFA"); // HEBREW LETTER TAV
	EnDeCode(CharacterSet::ISO8859_9, "\u011E", "\xD0"); // LATIN CAPITAL LETTER G WITH BREVE
	EnDeCode(CharacterSet::ISO8859_10, "\u0138", "\xFF"); // LATIN SMALL LETTER KRA
	EnDeCode(CharacterSet::ISO8859_11, "\u0E5B", "\xFB"); // THAI CHARACTER KHOMUT
	EnDeCode(CharacterSet::ISO8859_13, "\u2019", "\xFF"); // RIGHT SINGLE QUOTATION MARK
	EnDeCode(CharacterSet::ISO8859_14, "\u1E6B", "\xF7"); // LATIN SMALL LETTER T WITH DOT ABOVE
	EnDeCode(CharacterSet::ISO8859_15, "\u00BF", "\xBF"); // INVERTED QUESTION MARK
	EnDeCode(CharacterSet::ISO8859_16, "\u017C", "\xBF"); // LATIN SMALL LETTER Z WITH DOT ABOVE
//	EnDeCode(CharacterSet::Shift_JIS, "\u00A5", "\x5C"); // YEN SIGN Mapped to backslash
//	EnDeCode(CharacterSet::Shift_JIS, "\u203E", "\x7E"); // OVERLINE Mapped to tilde
	EnDeCode(CharacterSet::Shift_JIS, "\u3000", "\x81\x40"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::Cp1250, "\u20AC", "\x80"); // EURO SIGN
	EnDeCode(CharacterSet::Cp1251, "\u045F", "\x9F"); // CYRILLIC SMALL LETTER DZHE
	EnDeCode(CharacterSet::Cp1252, "\u02DC", "\x98"); // SMALL TILDE
	EnDeCode(CharacterSet::Cp1256, "\u0686", "\x8D"); // ARABIC LETTER TCHEH
	EnDeCode(CharacterSet::UTF16BE, "\u20AC", "\x20\xAC"); // EURO SIGN
	EnDeCode(CharacterSet::UTF8, "\u20AC", "\xE2\x82\xAC"); // EURO SIGN
	EnDeCode(CharacterSet::ASCII, "#", "#");
	EnDeCode(CharacterSet::Big5, "\u3000", "\xA1\x40"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::GB2312, "\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::EUC_KR, "\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
//	EnDeCode(CharacterSet::GBK, "\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::GB18030, "\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::UTF16LE, "\u20AC", "\xAC\x20"); // EURO SIGN
	EnDeCode(CharacterSet::UTF32BE, "\u20AC", std::string("\x00\x00\x20\xAC", 4)); // EURO SIGN
	EnDeCode(CharacterSet::UTF32LE, "\u20AC", std::string("\xAC\x20\x00\x00", 4)); // EURO SIGN
//	EnDeCode(CharacterSet::ISO646_Inv, "%", "%");
	EnDeCode(CharacterSet::BINARY, "\u0080\u00FF", "\x80\xFF");
	EnDeCode(CharacterSet::Unknown, "\u0080", "\x80"); // Treated as binary
	EnDeCode(CharacterSet::EUC_JP, "\u0080", "\x80"); // Not supported, treated as binary
}
