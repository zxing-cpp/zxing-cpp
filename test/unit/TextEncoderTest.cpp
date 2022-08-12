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

template <typename CharT>
void EnDeCode(CharacterSet cs, const CharT* in, std::string_view out)
{
	std::string bytes = TextEncoder::FromUnicode(reinterpret_cast<const char*>(in), cs);
	EXPECT_EQ(bytes, out);

	std::string dec;
	TextDecoder::Append(dec, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size(), cs);
	EXPECT_EQ(dec, reinterpret_cast<const char*>(in));
}

TEST(TextEncoderTest, FullCycleEncodeDecode)
{
	EnDeCode(CharacterSet::Cp437, u8"\u00C7", "\x80"); // LATIN CAPITAL LETTER C WITH CEDILLA
	EnDeCode(CharacterSet::ISO8859_1, u8"\u00A0", "\xA0"); // NO-BREAK SPACE
	EnDeCode(CharacterSet::ISO8859_2, u8"\u0104", "\xA1"); // LATIN CAPITAL LETTER A WITH OGONEK
	EnDeCode(CharacterSet::ISO8859_3, u8"\u0126", "\xA1"); // LATIN CAPITAL LETTER H WITH STROKE
	EnDeCode(CharacterSet::ISO8859_4, u8"\u0138", "\xA2"); // LATIN SMALL LETTER KRA
	EnDeCode(CharacterSet::ISO8859_5, u8"\u045F", "\xFF"); // CYRILLIC SMALL LETTER DZHE
	EnDeCode(CharacterSet::ISO8859_6, u8"\u0652", "\xF2"); // ARABIC SUKUN
	EnDeCode(CharacterSet::ISO8859_7, u8"\u03CE", "\xFE"); // GREEK SMALL LETTER OMEGA WITH TONOS
	EnDeCode(CharacterSet::ISO8859_8, u8"\u05EA", "\xFA"); // HEBREW LETTER TAV
	EnDeCode(CharacterSet::ISO8859_9, u8"\u011E", "\xD0"); // LATIN CAPITAL LETTER G WITH BREVE
	EnDeCode(CharacterSet::ISO8859_10, u8"\u0138", "\xFF"); // LATIN SMALL LETTER KRA
	EnDeCode(CharacterSet::ISO8859_11, u8"\u0E5B", "\xFB"); // THAI CHARACTER KHOMUT
	EnDeCode(CharacterSet::ISO8859_13, u8"\u2019", "\xFF"); // RIGHT SINGLE QUOTATION MARK
	EnDeCode(CharacterSet::ISO8859_14, u8"\u1E6B", "\xF7"); // LATIN SMALL LETTER T WITH DOT ABOVE
	EnDeCode(CharacterSet::ISO8859_15, u8"\u00BF", "\xBF"); // INVERTED QUESTION MARK
	EnDeCode(CharacterSet::ISO8859_16, u8"\u017C", "\xBF"); // LATIN SMALL LETTER Z WITH DOT ABOVE
//	EnDeCode(CharacterSet::Shift_JIS, u8"\u00A5", "\x5C"); // YEN SIGN Mapped to backslash
//	EnDeCode(CharacterSet::Shift_JIS, u8"\u203E", "\x7E"); // OVERLINE Mapped to tilde
	EnDeCode(CharacterSet::Shift_JIS, u8"\u3000", "\x81\x40"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::Cp1250, u8"\u20AC", "\x80"); // EURO SIGN
	EnDeCode(CharacterSet::Cp1251, u8"\u045F", "\x9F"); // CYRILLIC SMALL LETTER DZHE
	EnDeCode(CharacterSet::Cp1252, u8"\u02DC", "\x98"); // SMALL TILDE
	EnDeCode(CharacterSet::Cp1256, u8"\u0686", "\x8D"); // ARABIC LETTER TCHEH
	EnDeCode(CharacterSet::UTF16BE, u8"\u20AC", "\x20\xAC"); // EURO SIGN
	EnDeCode(CharacterSet::UTF8, u8"\u20AC", "\xE2\x82\xAC"); // EURO SIGN
	EnDeCode(CharacterSet::ASCII, u8"#", "#");
	EnDeCode(CharacterSet::Big5, u8"\u3000", "\xA1\x40"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::GB2312, u8"\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::EUC_KR, u8"\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
//	EnDeCode(CharacterSet::GBK, u8"\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::GB18030, u8"\u3000", "\xA1\xA1"); // IDEOGRAPHIC SPACE
	EnDeCode(CharacterSet::UTF16LE, u8"\u20AC", "\xAC\x20"); // EURO SIGN
	EnDeCode(CharacterSet::UTF32BE, u8"\u20AC", std::string("\x00\x00\x20\xAC", 4)); // EURO SIGN
	EnDeCode(CharacterSet::UTF32LE, u8"\u20AC", std::string("\xAC\x20\x00\x00", 4)); // EURO SIGN
//	EnDeCode(CharacterSet::ISO646_Inv, "%", "%");
	EnDeCode(CharacterSet::BINARY, u8"\u0080\u00FF", "\x80\xFF");
	EnDeCode(CharacterSet::Unknown, u8"\u0080", "\x80"); // Treated as binary
	EnDeCode(CharacterSet::EUC_JP, u8"\u0080", "\x80"); // Not supported, treated as binary
}
