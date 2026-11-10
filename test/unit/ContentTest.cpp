/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Content.h"
#include "ECI.h"
#include "Version.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace testing;

namespace std {
bool operator==(const string& lhs, const char8_t* rhs)
{
	return lhs == reinterpret_cast<const char*>(rhs);
}
} // namespace std

#if ZXING_ENABLE_UNICODE

TEST(ContentTest, Base)
{
	{ // Null
		Content c;
#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
		EXPECT_EQ(c.guessEncoding(), CharacterSet::Unknown);
#else
		EXPECT_EQ(c.guessEncoding(), CharacterSet::ISO8859_1);
#endif
		EXPECT_EQ(c.symbology.toString(), "");
		EXPECT_TRUE(c.empty());
	}

#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
	{ // set latin1
		Content c;
		c.switchEncoding(CharacterSet::ISO8859_1);
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u00E9Z");
#endif
		EXPECT_EQ(c.utf8(), u8"A\u00E9Z");
	}

	{ // set CharacterSet::ISO8859_5
		Content c;
		c.switchEncoding(CharacterSet::ISO8859_5);
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u0449Z");
#endif
		EXPECT_EQ(c.utf8(), u8"A\u0449Z");
	}

	{ // switch to CharacterSet::ISO8859_5
		Content c;
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u00E9Z");
#endif
		EXPECT_FALSE(c.hasECI);
		c.switchEncoding(CharacterSet::ISO8859_5);
		EXPECT_FALSE(c.hasECI);
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u0449Z");
#endif
		EXPECT_EQ(c.utf8(), u8"A\u00E9ZA\u0449Z");
	}
#endif // defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
}

#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
TEST(ContentTest, GuessEncoding)
{
	{ // guess latin1
		Content c;
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u00E9Z");
#endif
		EXPECT_EQ(c.guessEncoding(), CharacterSet::ISO8859_1);
		EXPECT_EQ(c.utf8(), u8"A\u00E9Z");
		EXPECT_EQ(c.bytesECI(), c.bytes);
	}

	{ // guess Shift_JIS
		Content c;
		c.append("A\x83\x65Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u30C6Z");
#endif
		EXPECT_EQ(c.guessEncoding(), CharacterSet::Shift_JIS);
		EXPECT_EQ(c.utf8(), u8"A\u30C6Z");
	}

	{
		auto expectEncoding = [](std::string_view bytes, CharacterSet expected) {
			Content c;
			c.append(bytes);
			EXPECT_EQ(c.guessEncoding(), expected);
		};

		// Short Latin-1 symbol strings should not be mistaken for half-width katakana.
		expectEncoding("\xAB" "Bonjour" "\xBB", CharacterSet::ISO8859_1);
		expectEncoding("3 m" "\xB2", CharacterSet::ISO8859_1);
		expectEncoding("\xB1\xB2", CharacterSet::ISO8859_1);

		// Shift_JIS evidence should still win for a longer kana run and valid double-byte text.
		expectEncoding("\xC3\xBD\xC4", CharacterSet::Shift_JIS);
		expectEncoding("\x93\x8C\x8B\x9E\x81\x40\x89\x77", CharacterSet::Shift_JIS);
		expectEncoding("\x81\xFC", CharacterSet::Shift_JIS);
		expectEncoding("\x81\x7F", CharacterSet::ISO8859_1);
		expectEncoding("\x81", CharacterSet::ISO8859_1);
		expectEncoding("\xF0\x40\xF1\x40\xF2\x40", CharacterSet::ISO8859_1);
	}
}
#endif // defined(ZXING_READERS) || defined(ZXING_USE_ZINT)

#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
TEST(ContentTest, ECI)
{
	{ // switch to ECI::ISO8859_5
		Content c;
		c.symbology = {'d', '1', 3}; // DataMatrix
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u00E9Z");
#endif
		c.switchEncoding(ECI::ISO8859_5);
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u0449Z");
#endif
		EXPECT_TRUE(c.hasECI);
		EXPECT_EQ(c.utf8(), u8"A\u00E9ZA\u0449Z");
		EXPECT_EQ(c.bytesECI().asString(), std::string_view("]d4\\000003A\xE9Z\\000007A\xE9Z"));
	}

	{ // switch ECI -> latin1 for unknown (instead of Shift_JIS)
		Content c;
		c.symbology = {'d', '1', 3}; // DataMatrix
		c.append("A\x83\x65Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u0083\u0065Z");
#endif
		c.switchEncoding(ECI::ISO8859_5);
		c.append("A\xE9Z");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("A\u0449Z");
#endif
		EXPECT_EQ(c.utf8(), u8"A\u0083\u0065ZA\u0449Z");
		EXPECT_EQ(c.bytesECI().asString(), std::string_view("]d4\\000003A\x83\x65Z\\000007A\xE9Z"));
	}

	{ // double '\'
		Content c;
		c.symbology = {'d', '1', 3}; // DataMatrix
		c.append("C:\\Test");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("C:\\Test");
#endif
		EXPECT_EQ(c.utf8(), u8"C:\\Test");
		EXPECT_EQ(c.bytesECI().asString(), std::string_view("]d1C:\\Test"));
		c.switchEncoding(ECI::UTF8);
		c.append("Täßt");
#ifndef ZXING_READERS
		c.utf8Cache.push_back("Täßt");
#endif
		EXPECT_EQ(c.bytesECI().asString(), std::string_view("]d4\\000003C:\\\\Test\\000026Täßt"));
	}
}
#endif // defined(ZXING_READERS) || defined(ZXING_USE_ZINT)

#endif // ZXING_ENABLE_UNICODE
