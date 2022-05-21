/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/rss/ODRSSFieldParser.h"

#include "DecodeStatus.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD::DataBar;

TEST(ODRSSFieldParserTest, ParseFieldsInGeneralPurpose)
{
	std::string result;

	// 2-digit AIs

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("00123456789012345678", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(00)123456789012345678");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("0012345678901234567", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("001234567890123456789", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("16123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(16)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("1612345", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("161234567", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("2212345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(22)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("221234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(22)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("22123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("9112345678901234567890123456789012345678901234567890"
				"1234567890123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(91)123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("9112345678901234567890123456789012345678901234567890"
				"123456789012345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(91)12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("9112345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("9912345678901234567890123456789012345678901234567890"
				"1234567890123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(99)123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("9912345678901234567890123456789012345678901234567890"
				"123456789012345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(99)12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("9912345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901", result), DecodeStatus::NotFound);

	// 3-digit AIs

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("2351234567890123456789012345678", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(235)1234567890123456789012345678");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("235123456789012345678901234567", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(235)123456789012345678901234567");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("23512345678901234567890123456789", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("24312345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(243)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("2431234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(243)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("243123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("253123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(253)123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("25312345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(253)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("2531234567890123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("2551234567890123456789012345", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(255)1234567890123456789012345");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("255123456789012345678901234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(255)123456789012345678901234");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("25512345678901234567890123456", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4151234567890123", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(415)1234567890123");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("415123456789012", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("41512345678901234", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4171234567890123", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(417)1234567890123");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("417123456789012", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("41712345678901234", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("421123456789012", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(421)123456789012");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("42112345678901", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(421)12345678901");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4211234567890123", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("425123456789012345", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(425)123456789012345");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("42512345678901234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(425)12345678901234");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4251234567890123456", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("427123", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(427)123");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("42712", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(427)12");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4271234", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("71012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(710)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7101234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(710)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("710123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("71512345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(715)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7151234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(715)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("715123456789012345678901", result), DecodeStatus::NotFound);

	// 4-digit variable 4th

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("3370123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(3370)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("337012345", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("33701234567", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("3375123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(3375)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("33751234567", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("337512345", result), DecodeStatus::NotFound);

	EXPECT_EQ(ParseFieldsInGeneralPurpose("3376123456", result), DecodeStatus::NoError); // Allow although > max 3375

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("39401234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(3940)1234");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("394012345", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("3940123", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("39431234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(3943)1234");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("394312345", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("3943123", result), DecodeStatus::NotFound);

	EXPECT_EQ(ParseFieldsInGeneralPurpose("39441234", result), DecodeStatus::NoError); // Allow although > max 3943

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("3950123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(3950)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("39501234567", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("395012345", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("3955123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(3955)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("39551234567", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("395512345", result), DecodeStatus::NotFound);

	EXPECT_EQ(ParseFieldsInGeneralPurpose("3956123456", result), DecodeStatus::NoError); // Allow although > max 3955

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7230123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7230)123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("723012345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7230)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("72301234567890123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7239123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7239)123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("723912345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7239)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("72391234567890123456789012345678901", result), DecodeStatus::NotFound);

	// 4-digit AIs

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("430012345678901234567890123456789012345", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4300)12345678901234567890123456789012345");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43001234567890123456789012345678901234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4300)1234567890123456789012345678901234");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4300123456789012345678901234567890123456", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("430712", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4307)12");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4307123", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43071", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4308123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4308)123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("430812345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4308)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43081234567890123456789012345678901", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("431712", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4317)12");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4317123", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43171", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("431812345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4318)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43181234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4318)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4318123456789012345678901", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43211", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4321)1");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("432112", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4321", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("4326123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(4326)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("43261234567", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("432612345", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("70041234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7004)1234");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7004123", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7004)123");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("700412345", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7006123456", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7006)123456");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("70061234567", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("700612345", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("701012", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7010)12");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("70101", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7010)1");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7010123", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("702012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7020)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("70201234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7020)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7020123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7023123456789012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7023)123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("702312345678901234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7023)12345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("70231234567890123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("70401234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7040)1234");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("704012345", result), DecodeStatus::NotFound);
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7040123", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("724012345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7240)12345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("72401234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(7240)1234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("7240123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("80071234567890123456789012345678901234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8007)1234567890123456789012345678901234");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8007123456789012345678901234567890123", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8007)123456789012345678901234567890123");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("800712345678901234567890123456789012345", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("800912345678901234567890123456789012345678901234567890", result),
				DecodeStatus::NoError);
	EXPECT_EQ(result, "(8009)12345678901234567890123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("80091234567890123456789012345678901234567890123456789", result),
				DecodeStatus::NoError);
	EXPECT_EQ(result, "(8009)1234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8009123456789012345678901234567890123456789012345678901", result),
				DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("80131234567890123456789012345", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8013)1234567890123456789012345");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8013123456789012345678901234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8013)123456789012345678901234");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("801312345678901234567890123456", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8017123456789012345678", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8017)123456789012345678");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("80171234567890123456789", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("801712345678901234567", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("80191234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8019)1234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8019123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8019)123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("801912345678901", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8026123456789012345678", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8026)123456789012345678");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("80261234567890123456789", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("802612345678901234567", result), DecodeStatus::NotFound);

	// Non-existing
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8100123456", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("81011234567890", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("810212", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811012345678901234567890123456789012345678901234567890"
				"12345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8110)1234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811012345678901234567890123456789012345678901234567890"
				"1234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8110)123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811012345678901234567890123456789012345678901234567890"
				"123456789012345678901", result), DecodeStatus::NotFound);

	// Fixed length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("81111234", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8111)1234");
	// Incorrect lengths
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811112345", result), DecodeStatus::NotFound);
	EXPECT_EQ(ParseFieldsInGeneralPurpose("8111123", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811212345678901234567890123456789012345678901234567890"
				"12345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8112)1234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811212345678901234567890123456789012345678901234567890"
				"1234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8112)123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("811212345678901234567890123456789012345678901234567890"
				"123456789012345678901", result), DecodeStatus::NotFound);

	// Max length
	EXPECT_EQ(ParseFieldsInGeneralPurpose("820012345678901234567890123456789012345678901234567890"
				"12345678901234567890", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8200)1234567890123456789012345678901234567890123456789012345678901234567890");
	EXPECT_EQ(ParseFieldsInGeneralPurpose("820012345678901234567890123456789012345678901234567890"
				"1234567890123456789", result), DecodeStatus::NoError);
	EXPECT_EQ(result, "(8200)123456789012345678901234567890123456789012345678901234567890123456789");
	// Too long
	EXPECT_EQ(ParseFieldsInGeneralPurpose("820012345678901234567890123456789012345678901234567890"
				"123456789012345678901", result), DecodeStatus::NotFound);
}
