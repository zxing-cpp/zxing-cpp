/*
 * Copyright 2022 gitlost
 */
// SPDX-License-Identifier: Apache-2.0

#include "GS1.h"

#include "gtest/gtest.h"

using namespace ZXing;

TEST(HRIFromGS1, Single)
{
	// 2-digit AIs

	// Fixed length
	EXPECT_EQ(HRIFromGS1("00123456789012345678"), "(00)123456789012345678");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("0012345678901234567"), "");
	EXPECT_EQ(HRIFromGS1("001234567890123456789"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("16123456"), "(16)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("1612345"), "");
	EXPECT_EQ(HRIFromGS1("161234567"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("2212345678901234567890"), "(22)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("221234567890123456789"), "(22)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("22123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("91123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
			  "(91)123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("9112345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"),
			  "(91)12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("911234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("99123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
			  "(99)123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("9912345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"),
			  "(99)12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("991234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"), "");

	// 3-digit AIs

	EXPECT_EQ(HRIFromGS1("310"), ""); // incomplete prefix

	// Max length
	EXPECT_EQ(HRIFromGS1("2351234567890123456789012345678"), "(235)1234567890123456789012345678");
	EXPECT_EQ(HRIFromGS1("235123456789012345678901234567"), "(235)123456789012345678901234567");
	// Too long
	EXPECT_EQ(HRIFromGS1("23512345678901234567890123456789"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("24312345678901234567890"), "(243)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("2431234567890123456789"), "(243)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("243123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("253123456789012345678901234567890"), "(253)123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("25312345678901234567890123456789"), "(253)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("2531234567890123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("2551234567890123456789012345"), "(255)1234567890123456789012345");
	EXPECT_EQ(HRIFromGS1("255123456789012345678901234"), "(255)123456789012345678901234");
	// Too long
	EXPECT_EQ(HRIFromGS1("25512345678901234567890123456"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("4151234567890123"), "(415)1234567890123");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("415123456789012"), "");
	EXPECT_EQ(HRIFromGS1("41512345678901234"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("4171234567890123"), "(417)1234567890123");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("417123456789012"), "");
	EXPECT_EQ(HRIFromGS1("41712345678901234"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("421123456789012"), "(421)123456789012");
	EXPECT_EQ(HRIFromGS1("42112345678901"), "(421)12345678901");
	// Too long
	EXPECT_EQ(HRIFromGS1("4211234567890123"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("425123456789012345"), "(425)123456789012345");
	EXPECT_EQ(HRIFromGS1("42512345678901234"), "(425)12345678901234");
	// Too long
	EXPECT_EQ(HRIFromGS1("4251234567890123456"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("427123"), "(427)123");
	EXPECT_EQ(HRIFromGS1("42712"), "(427)12");
	// Too long
	EXPECT_EQ(HRIFromGS1("4271234"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("71012345678901234567890"), "(710)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("7101234567890123456789"), "(710)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("710123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("71512345678901234567890"), "(715)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("7151234567890123456789"), "(715)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("715123456789012345678901"), "");

	// 4-digit variable 4th

	// Fixed length
	EXPECT_EQ(HRIFromGS1("3370123456"), "(3370)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("337012345"), "");
	EXPECT_EQ(HRIFromGS1("33701234567"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("3375123456"), "(3375)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("33751234567"), "");
	EXPECT_EQ(HRIFromGS1("337512345"), "");

	//	EXPECT_EQ(ParseFieldsInGeneralPurpose("3376123456"),  // Allow although > max 3375

	// Fixed length
	EXPECT_EQ(HRIFromGS1("39401234"), "(3940)1234");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("394012345"), "");
	EXPECT_EQ(HRIFromGS1("3940123"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("39431234"), "(3943)1234");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("394312345"), "");
	EXPECT_EQ(HRIFromGS1("3943123"), "");

	//	EXPECT_EQ(ParseFieldsInGeneralPurpose("39441234"),  // Allow although > max 3943

	// Fixed length
	EXPECT_EQ(HRIFromGS1("3950123456"), "(3950)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("39501234567"), "");
	EXPECT_EQ(HRIFromGS1("395012345"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("3955123456"), "(3955)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("39551234567"), "");
	EXPECT_EQ(HRIFromGS1("395512345"), "");

	//	EXPECT_EQ(ParseFieldsInGeneralPurpose("3956123456"), // Allow although > max 3955

	// Max length
	EXPECT_EQ(HRIFromGS1("7230123456789012345678901234567890"), "(7230)123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("723012345678901234567890123456789"), "(7230)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("72301234567890123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("7239123456789012345678901234567890"), "(7239)123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("723912345678901234567890123456789"), "(7239)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("72391234567890123456789012345678901"), "");

	// 4-digit AIs

	// Max length
	EXPECT_EQ(HRIFromGS1("430012345678901234567890123456789012345"), "(4300)12345678901234567890123456789012345");
	EXPECT_EQ(HRIFromGS1("43001234567890123456789012345678901234"), "(4300)1234567890123456789012345678901234");
	// Too long
	EXPECT_EQ(HRIFromGS1("4300123456789012345678901234567890123456"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("430712"), "(4307)12");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("4307123"), "");
	EXPECT_EQ(HRIFromGS1("43071"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("4308123456789012345678901234567890"), "(4308)123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("430812345678901234567890123456789"), "(4308)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("43081234567890123456789012345678901"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("431712"), "(4317)12");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("4317123"), "");
	EXPECT_EQ(HRIFromGS1("43171"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("431812345678901234567890"), "(4318)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("43181234567890123456789"), "(4318)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("4318123456789012345678901"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("43211"), "(4321)1");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("432112"), "");
	EXPECT_EQ(HRIFromGS1("4321"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("4326123456"), "(4326)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("43261234567"), "");
	EXPECT_EQ(HRIFromGS1("432612345"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("70041234"), "(7004)1234");
	EXPECT_EQ(HRIFromGS1("7004123"), "(7004)123");
	// Too long
	EXPECT_EQ(HRIFromGS1("700412345"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("7006123456"), "(7006)123456");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("70061234567"), "");
	EXPECT_EQ(HRIFromGS1("700612345"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("701012"), "(7010)12");
	EXPECT_EQ(HRIFromGS1("70101"), "(7010)1");
	// Too long
	EXPECT_EQ(HRIFromGS1("7010123"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("702012345678901234567890"), "(7020)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("70201234567890123456789"), "(7020)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("7020123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("7023123456789012345678901234567890"), "(7023)123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("702312345678901234567890123456789"), "(7023)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("70231234567890123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("70401234"), "(7040)1234");
	EXPECT_EQ(HRIFromGS1("704012345"), "");
	// Too long
	EXPECT_EQ(HRIFromGS1("7040123"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("724012345678901234567890"), "(7240)12345678901234567890");
	EXPECT_EQ(HRIFromGS1("72401234567890123456789"), "(7240)1234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("7240123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("80071234567890123456789012345678901234"), "(8007)1234567890123456789012345678901234");
	EXPECT_EQ(HRIFromGS1("8007123456789012345678901234567890123"), "(8007)123456789012345678901234567890123");
	// Too long
	EXPECT_EQ(HRIFromGS1("800712345678901234567890123456789012345"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("800912345678901234567890123456789012345678901234567890"),

			  "(8009)12345678901234567890123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("80091234567890123456789012345678901234567890123456789"),

			  "(8009)1234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("8009123456789012345678901234567890123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("80131234567890123456789012345"), "(8013)1234567890123456789012345");
	EXPECT_EQ(HRIFromGS1("8013123456789012345678901234"), "(8013)123456789012345678901234");
	// Too long
	EXPECT_EQ(HRIFromGS1("801312345678901234567890123456"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("8017123456789012345678"), "(8017)123456789012345678");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("80171234567890123456789"), "");
	EXPECT_EQ(HRIFromGS1("801712345678901234567"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("80191234567890"), "(8019)1234567890");
	EXPECT_EQ(HRIFromGS1("8019123456789"), "(8019)123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("801912345678901"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("8026123456789012345678"), "(8026)123456789012345678");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("80261234567890123456789"), "");
	EXPECT_EQ(HRIFromGS1("802612345678901234567"), "");

	// Non-existing
	EXPECT_EQ(HRIFromGS1("8100123456"), "");
	EXPECT_EQ(HRIFromGS1("81011234567890"), "");
	EXPECT_EQ(HRIFromGS1("810212"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("81101234567890123456789012345678901234567890123456789012345678901234567890"),
			  "(8110)1234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("8110123456789012345678901234567890123456789012345678901234567890123456789"),
			  "(8110)123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("811012345678901234567890123456789012345678901234567890123456789012345678901"), "");

	// Fixed length
	EXPECT_EQ(HRIFromGS1("81111234"), "(8111)1234");
	// Incorrect lengths
	EXPECT_EQ(HRIFromGS1("811112345"), "");
	EXPECT_EQ(HRIFromGS1("8111123"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("81121234567890123456789012345678901234567890123456789012345678901234567890"),
			  "(8112)1234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("8112123456789012345678901234567890123456789012345678901234567890123456789"),
			  "(8112)123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("811212345678901234567890123456789012345678901234567890123456789012345678901"), "");

	// Max length
	EXPECT_EQ(HRIFromGS1("82001234567890123456789012345678901234567890123456789012345678901234567890"),
			  "(8200)1234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(HRIFromGS1("8200123456789012345678901234567890123456789012345678901234567890123456789"),
			  "(8200)123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(HRIFromGS1("820012345678901234567890123456789012345678901234567890123456789012345678901"), "");
}

TEST(HRIFromGS1, MultiFixed)
{
	EXPECT_EQ(HRIFromGS1("81111234430712"), "(8111)1234(4307)12");
}

TEST(HRIFromGS1, MultiVariable)
{
	EXPECT_EQ(HRIFromGS1("70041234\x1d""81111234"), "(7004)1234(8111)1234");
}
