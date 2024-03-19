/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "GTIN.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::GTIN;

TEST(GTINTest, CountryIdentifierEAN13)
{
	// From test/samples/ean13-*/
	EXPECT_EQ(LookupCountryIdentifier("8413000065504"), "ES");
	EXPECT_EQ(LookupCountryIdentifier("8413000065504 12"), "ES");
	EXPECT_EQ(LookupCountryIdentifier("8413000065504 51299"), "ES");
	EXPECT_EQ(LookupCountryIdentifier("5449000039231"), "BE");
	EXPECT_TRUE(LookupCountryIdentifier("9788430532674").empty()); // Bookland (ISBN)
	EXPECT_EQ(LookupCountryIdentifier("8480017507990"), "ES");
	EXPECT_EQ(LookupCountryIdentifier("3166298099809"), "FR");
	EXPECT_EQ(LookupCountryIdentifier("5201815331227"), "GR");
	EXPECT_EQ(LookupCountryIdentifier("3560070169443"), "FR");
	EXPECT_EQ(LookupCountryIdentifier("4045787034318"), "DE");
	EXPECT_EQ(LookupCountryIdentifier("3086126100326"), "FR");
	EXPECT_EQ(LookupCountryIdentifier("4820024790635"), "UA");
	EXPECT_EQ(LookupCountryIdentifier("7622200008018"), "CH");
	EXPECT_EQ(LookupCountryIdentifier("5603667020517"), "PT");
	EXPECT_EQ(LookupCountryIdentifier("5709262942503"), "DK");
	EXPECT_EQ(LookupCountryIdentifier("4901780188352"), "JP");
	EXPECT_EQ(LookupCountryIdentifier("4007817327098"), "DE");
	EXPECT_EQ(LookupCountryIdentifier("5025121072311"), "GB");
	EXPECT_EQ(LookupCountryIdentifier("5025121072311 12"), "GB");
	EXPECT_EQ(LookupCountryIdentifier("5025121072311 51299"), "GB");
	EXPECT_EQ(LookupCountryIdentifier("5030159003930"), "GB");
	EXPECT_EQ(LookupCountryIdentifier("5000213002834"), "GB");
	EXPECT_TRUE(LookupCountryIdentifier("1920081045006").empty()); // 140-199 unassigned
	EXPECT_TRUE(LookupCountryIdentifier("9780735200449 51299").empty()); // Bookland (ISBN)

	// Other
	EXPECT_TRUE(LookupCountryIdentifier("0000000001465").empty()); // 0000000 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("0000000111461 12").empty());
	EXPECT_TRUE(LookupCountryIdentifier("0000001991469").empty()); // 0000001-0000099 unused to avoid GTIN-8 collision
	EXPECT_TRUE(LookupCountryIdentifier("0000099991463").empty());
	EXPECT_EQ(LookupCountryIdentifier("0000102301463"), "US"); // 00001-00009 US
	EXPECT_EQ(LookupCountryIdentifier("0000102301463 51299"), "US");
	EXPECT_EQ(LookupCountryIdentifier("0000902301465"), "US");
	EXPECT_EQ(LookupCountryIdentifier("0001602301465"), "US"); // 0001-0009 US
	EXPECT_EQ(LookupCountryIdentifier("0009602301461 12"), "US");
	EXPECT_EQ(LookupCountryIdentifier("0016602301469"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("0036602301467"), "US");
	EXPECT_EQ(LookupCountryIdentifier("0196602301468 51299"), "US");
	EXPECT_TRUE(LookupCountryIdentifier("0206602301464").empty()); // 020-029 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("0296602301465").empty());
	EXPECT_EQ(LookupCountryIdentifier("0306602301461"), "US"); // 030-039 US
	EXPECT_EQ(LookupCountryIdentifier("0396602301462"), "US");
	EXPECT_TRUE(LookupCountryIdentifier("0406602301468").empty()); // 040-049 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("0496602301469").empty());
	EXPECT_TRUE(LookupCountryIdentifier("0506602301465").empty()); // 050-059 reserved for future use
	EXPECT_TRUE(LookupCountryIdentifier("0596602301466").empty());
	EXPECT_EQ(LookupCountryIdentifier("0606602301462"), "US"); // 060-099 US
	EXPECT_EQ(LookupCountryIdentifier("0996602301464"), "US");
	EXPECT_EQ(LookupCountryIdentifier("1006602301469"), "US"); // 100-139 US
	EXPECT_EQ(LookupCountryIdentifier("1396602301461"), "US");
	EXPECT_TRUE(LookupCountryIdentifier("1406602301467").empty()); // 140-199 unassigned
	EXPECT_TRUE(LookupCountryIdentifier("1996602301463").empty());
	EXPECT_TRUE(LookupCountryIdentifier("2006602301468").empty()); // 200-299 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("2996602301462").empty());
	EXPECT_EQ(LookupCountryIdentifier("9586602301468"), "MO");
	EXPECT_EQ(LookupCountryIdentifier("9586602301468 12"), "MO");
	EXPECT_EQ(LookupCountryIdentifier("9586602301468 51299"), "MO");

	// Additions/updates
	EXPECT_EQ(LookupCountryIdentifier("3890102301467"), "ME");
	//EXPECT_EQ(LookupCountryIdentifier("3900102301463"), "XK"); // Kosovo according to Wikipedia - awaiting GS1 confirmation
	EXPECT_EQ(LookupCountryIdentifier("4700102301468"), "KG");
	EXPECT_EQ(LookupCountryIdentifier("4830102301462"), "TM");
	EXPECT_EQ(LookupCountryIdentifier("4880102301467"), "TJ");
	EXPECT_EQ(LookupCountryIdentifier("5210102301461"), "GR");
	EXPECT_EQ(LookupCountryIdentifier("5300102301469"), "AL");
	EXPECT_EQ(LookupCountryIdentifier("6040102301463"), "SN");
	EXPECT_EQ(LookupCountryIdentifier("6150102301469"), "NG");
	EXPECT_EQ(LookupCountryIdentifier("6170102301467"), "CM");
	EXPECT_EQ(LookupCountryIdentifier("6200102301461"), "TZ");
	EXPECT_EQ(LookupCountryIdentifier("6230102301468"), "BN");
	EXPECT_EQ(LookupCountryIdentifier("6300102301468"), "QA");
	EXPECT_EQ(LookupCountryIdentifier("6310102301467"), "NA");
	EXPECT_EQ(LookupCountryIdentifier("6990102301461"), "CN");
	EXPECT_EQ(LookupCountryIdentifier("7710102301464"), "CO");
	EXPECT_EQ(LookupCountryIdentifier("7780102301467"), "AR");
	EXPECT_TRUE(LookupCountryIdentifier("7850102301467").empty());
	EXPECT_EQ(LookupCountryIdentifier("8600102301467"), "RS");
	EXPECT_EQ(LookupCountryIdentifier("8830102301468"), "MM");
	EXPECT_EQ(LookupCountryIdentifier("8840102301467"), "KH");
	EXPECT_EQ(LookupCountryIdentifier("9400102301462"), "NZ");
}

TEST(GTINTest, CountryIdentifierUPCA)
{
	// From test/samples/upca-*/
	EXPECT_EQ(LookupCountryIdentifier("036602301467"), "US"); // 001-019 US/CA
	EXPECT_EQ(LookupCountryIdentifier("036602301467 12"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("036602301467 51299"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("070097025088"), "US");
	EXPECT_EQ(LookupCountryIdentifier("781735802045"), "US"); // 060-099 US
	EXPECT_TRUE(LookupCountryIdentifier("456314319671").empty()); // 040-049 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("434704791429").empty());
	EXPECT_EQ(LookupCountryIdentifier("752919460009"), "US"); // 060-099 US
	EXPECT_EQ(LookupCountryIdentifier("606949762520"), "US");
	EXPECT_EQ(LookupCountryIdentifier("890444000335"), "US");
	EXPECT_EQ(LookupCountryIdentifier("181497000879"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("012546619592"), "US");
	EXPECT_EQ(LookupCountryIdentifier("854818000116"), "US"); // 060-099 US
	EXPECT_EQ(LookupCountryIdentifier("312547701310"), "US"); // 030-039 US
	EXPECT_EQ(LookupCountryIdentifier("071831007995 19868"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("027011006951 02601"), "US");
	EXPECT_EQ(LookupCountryIdentifier("024543136538 00"), "US");

	// Other
	EXPECT_TRUE(LookupCountryIdentifier("000000001465").empty()); // 0000000 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("000000111461 12").empty());
	EXPECT_TRUE(LookupCountryIdentifier("000001991468").empty()); // 0000001-0000099 unused to avoid GTIN-8 collision
	EXPECT_TRUE(LookupCountryIdentifier("000099991463").empty());
	EXPECT_EQ(LookupCountryIdentifier("000102301463"), "US"); // 00001-00009 US
	EXPECT_EQ(LookupCountryIdentifier("000102301463 51299"), "US");
	EXPECT_EQ(LookupCountryIdentifier("000902301465"), "US");
	EXPECT_EQ(LookupCountryIdentifier("001602301465"), "US"); // 0001-0009 US
	EXPECT_EQ(LookupCountryIdentifier("009602301461 12"), "US");
	EXPECT_EQ(LookupCountryIdentifier("016602301469"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("036602301467"), "US");
	EXPECT_EQ(LookupCountryIdentifier("196602301468 51299"), "US");
	EXPECT_TRUE(LookupCountryIdentifier("206602301464").empty()); // 020-029 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("296602301465").empty());
	EXPECT_EQ(LookupCountryIdentifier("306602301461"), "US"); // 030-039 US
	EXPECT_EQ(LookupCountryIdentifier("396602301462"), "US");
	EXPECT_TRUE(LookupCountryIdentifier("406602301468").empty()); // 040-049 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("496602301469").empty());
	EXPECT_TRUE(LookupCountryIdentifier("506602301465").empty()); // 050-059 reserved for future use
	EXPECT_TRUE(LookupCountryIdentifier("596602301466").empty());
	EXPECT_EQ(LookupCountryIdentifier("606602301462"), "US"); // 060-099 US
	EXPECT_EQ(LookupCountryIdentifier("996602301464"), "US");
}

TEST(GTINTest, CountryIdentifierUPCE)
{
	// From test/samples/upce-*/
	EXPECT_EQ(LookupCountryIdentifier("01234565"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("01234565", BarcodeFormat::UPCE), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("00123457"), "US"); // 0001-0009 US
	EXPECT_EQ(LookupCountryIdentifier("00123457", BarcodeFormat::UPCE), "US"); // 0001-0009 US
	EXPECT_EQ(LookupCountryIdentifier("05096893"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("05096893", BarcodeFormat::UPCE), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("04963406 01"), "US"); // 001-019 US
	EXPECT_EQ(LookupCountryIdentifier("04963406 01", BarcodeFormat::UPCE), "US"); // 001-019 US

	// Other
	// 0000000, 0000001-0000099 and 00001-00009 not possible for UPC-E
	EXPECT_EQ(LookupCountryIdentifier("00021357"), "US"); // 0001-0009 US
	EXPECT_EQ(LookupCountryIdentifier("00021357 01"), "US");
	EXPECT_EQ(LookupCountryIdentifier("11621355"), "US"); // 001-019 US
	EXPECT_TRUE(LookupCountryIdentifier("22221111").empty()); // 020-029 Restricted Circulation Numbers
	EXPECT_EQ(LookupCountryIdentifier("31621358"), "US"); // 030-039 US
	EXPECT_TRUE(LookupCountryIdentifier("40621359").empty()); // 040-049 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("50621359").empty()); // 050-059 reserved for future use
	EXPECT_EQ(LookupCountryIdentifier("61621358"), "US"); // 060-099 US
	EXPECT_EQ(LookupCountryIdentifier("99621350"), "US");
}

TEST(GTINTest, CountryIdentifierEAN8)
{
	auto format = BarcodeFormat::EAN8; // Require BarcodeFormat for EAN-8 to be distinguished from UPC-E

	// From test/samples/ean8-*/
	EXPECT_EQ(LookupCountryIdentifier("48512343", format), "AM");
	EXPECT_EQ(LookupCountryIdentifier("12345670", format), "US");
	EXPECT_TRUE(LookupCountryIdentifier("67678983", format).empty()); // 650-689 unassigned
	EXPECT_EQ(LookupCountryIdentifier("80674313", format), "IT");
	EXPECT_EQ(LookupCountryIdentifier("59001270", format), "PL");
	EXPECT_EQ(LookupCountryIdentifier("50487066", format), "GB");
	EXPECT_TRUE(LookupCountryIdentifier("55123457", format).empty()); // 550-559 unassigned
	EXPECT_TRUE(LookupCountryIdentifier("95012346", format).empty()); // 950 GS1 Global Office

	// Other (GS1 General Specifications 1.4.3 Figure 1.4.3-1
	EXPECT_TRUE(LookupCountryIdentifier("00045674", format).empty()); // 000-099 EAN-8 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("09945678", format).empty());
	EXPECT_EQ(LookupCountryIdentifier("10045671", format), "US"); // 100-139 US
	EXPECT_EQ(LookupCountryIdentifier("13945671", format), "US");
	EXPECT_TRUE(LookupCountryIdentifier("14045677", format).empty()); // 140-199 unassigned
	EXPECT_TRUE(LookupCountryIdentifier("19945675", format).empty());
	EXPECT_TRUE(LookupCountryIdentifier("20045678", format).empty()); // 200-299 Restricted Circulation Numbers
	EXPECT_TRUE(LookupCountryIdentifier("29945672", format).empty());
	EXPECT_EQ(LookupCountryIdentifier("30045675", format), "FR");
	EXPECT_EQ(LookupCountryIdentifier("95845678", format), "MO");
	EXPECT_TRUE(LookupCountryIdentifier("97645672", format).empty()); // Unassigned
	EXPECT_TRUE(LookupCountryIdentifier("97745679", format).empty()); // 977-999 Reserved for future use
	EXPECT_TRUE(LookupCountryIdentifier("99945671", format).empty());
}

TEST(GTINTest, CountryIdentifierGTIN14)
{
	// From test/samples/itf-*/
	EXPECT_EQ(LookupCountryIdentifier("30712345000010"), "US");
	EXPECT_EQ(LookupCountryIdentifier("00012345678905"), "US");

	// Other
	EXPECT_TRUE(LookupCountryIdentifier("12345678901231").empty()); // 200-299 Restricted Circulation Numbers
	EXPECT_EQ(LookupCountryIdentifier("13005678901233"), "FR");
}
