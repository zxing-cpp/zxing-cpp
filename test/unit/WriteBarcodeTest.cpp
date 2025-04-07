/*
 * Copyright 2025 gitlost
 * Copyright 2025 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "Version.h"

#if defined(ZXING_EXPERIMENTAL_API)

#ifdef ZXING_READERS
#include "ReadBarcode.h"
#endif
#include "WriteBarcode.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace testing;

static void check(int line, std::string_view input, CreatorOptions cOpts, std::string_view symbologyIdentifier, std::string_view text,
				  std::string_view bytes, bool hasECI, std::string_view textECI, std::string_view bytesECI, std::string_view HRI,
				  std::string_view contentType, std::string_view position = {}, std::string_view ecLevel = {},
				  std::string_view version = {})
{
	auto bc = CreateBarcodeFromText(input, cOpts);

	EXPECT_TRUE(bc.isValid()) << "line:" << line;
	EXPECT_EQ(bc.symbologyIdentifier(), symbologyIdentifier) << "line:" << line;
	EXPECT_EQ(ToString(bc.contentType()), contentType) << "line:" << line;
	EXPECT_EQ(bc.text(TextMode::HRI), HRI) << "line:" << line;
#ifdef ZXING_READERS
	EXPECT_EQ(bc.text(TextMode::Plain), text) << "line:" << line;
	EXPECT_EQ(ToHex(bc.bytes()), bytes) << "line:" << line;
	EXPECT_EQ(bc.hasECI(), hasECI) << "line:" << line;
	EXPECT_EQ(bc.text(TextMode::ECI), textECI) << "line:" << line;
	EXPECT_EQ(ToHex(bc.bytesECI()), bytesECI) << "line:" << line;
	// if (!position.empty())
	// 	EXPECT_EQ(ToString(bc.position()), position) << "line:" << line;
	// EXPECT_EQ(bc.ecLevel(), ecLevel) << "line:" << line;
	// EXPECT_EQ(bc.version(), version) << "line:" << line;

	auto br = ReadBarcode(bc.symbol(), ReaderOptions().setFormats(bc.format()).setIsPure(true));

	EXPECT_EQ(bc.isValid(), br.isValid()) << "line:" << line;
	EXPECT_EQ(ToString(bc.format()), ToString(br.format())) << "line:" << line;
	EXPECT_EQ(bc.symbologyIdentifier(), br.symbologyIdentifier()) << "line:" << line;
	EXPECT_EQ(bc.text(TextMode::Plain), br.text(TextMode::Plain)) << "line:" << line;
	EXPECT_EQ(ToHex(bc.bytes()), ToHex(br.bytes())) << "line:" << line;
	EXPECT_EQ(bc.hasECI(), br.hasECI()) << "line:" << line;
	EXPECT_EQ(bc.text(TextMode::ECI), br.text(TextMode::ECI)) << "line:" << line;
	EXPECT_EQ(ToHex(bc.bytesECI()), ToHex(br.bytesECI())) << "line:" << line;
	EXPECT_EQ(bc.text(TextMode::HRI), br.text(TextMode::HRI)) << "line:" << line;
	EXPECT_EQ(ToString(bc.contentType()), ToString(br.contentType())) << "line:" << line;
	// EXPECT_EQ(ToString(bc.position()), ToString(br.position())) << "line:" << line;
	// EXPECT_EQ(bc.ecLevel(), br.ecLevel()) << "line:" << line;
	// EXPECT_EQ(bc.version(), br.version()) << "line:" << line;

	EXPECT_EQ(bc.orientation(), br.orientation()) << "line:" << line;
	EXPECT_EQ(bc.isMirrored(), br.isMirrored()) << "line:" << line;
	EXPECT_EQ(bc.isInverted(), br.isInverted()) << "line:" << line;
	EXPECT_EQ(bc.readerInit(), br.readerInit()) << "line:" << line;
#endif
}

TEST(WriteBarcodeTest, ZintASCII)
{
	check(__LINE__, "1234", BarcodeFormat::Aztec, "]z0", "1234", "31 32 33 34", false, "]z3\\0000261234", "5D 7A 33 31 32 33 34",
		  "1234", "Text", "0x0 15x0 15x15 0x15", "58%", "1" /*version*/);

	check(__LINE__, "A12B", BarcodeFormat::Codabar, "]F0", "A12B", "41 31 32 42", false, "]F0\\000026A12B", "5D 46 30 41 31 32 42",
		  "A12B", "Text", "0x0 40x0 40x49 0x49");

	// check(__LINE__, "1234", BarcodeFormat::CodablockF, "]O4", "1234", "31 32 33 34", false, "]O4\\0000261234",
	// 	  "5D 4F 34 31 32 33 34", "1234", "Text", "0x0 100x0 100x21 0x21");

	check(__LINE__, "1234", BarcodeFormat::Code128, "]C0", "1234", "31 32 33 34", false, "]C0\\0000261234", "5D 43 30 31 32 33 34",
		  "1234", "Text", "0x0 56x0 56x49 0x49");

	// check(__LINE__, "1234", BarcodeFormat::Code16K, "]K0", "1234", "31 32 33 34", false, "]K0\\0000261234",
	// 	  "5D 4B 30 31 32 33 34", "1234", "Text", "0x0 69x0 69x23 0x23");

	// Plain (non-extended) Code 39
	check(__LINE__, "1234", BarcodeFormat::Code39, "]A0", "1234", "31 32 33 34", false, "]A0\\0000261234", "5D 41 30 31 32 33 34",
		  "1234", "Text", "0x0 76x0 76x49 0x49");

	// Extended Code 39 with DEL
	// HRI not escaped as content type considered "Text" (DEL not recognized)
	// check(__LINE__, "12\17734", BarcodeFormat::Code39, "]A4", "12\17734", "31 32 7F 33 34", false, "]A4\\00002612\17734",
	// 	  "5D 41 34 31 32 7F 33 34", "12\17734", "Text", "0x0 102x0 102x49 0x49");

	// Extended Code 39 with SOH & DEL
	// HRI escaped as content type considered "Binary" (SOH)
	// check(__LINE__, "12\001\17734", BarcodeFormat::Code39, "]A4", "12\001\17734", "31 32 01 7F 33 34", false,
	// 	  "]A4\\00002612\001\17734", "5D 41 34 31 32 01 7F 33 34", "12<SOH><DEL>34", "Binary", "0x0 128x0 128x49 0x49");

	// Extended Code 39 with NUL
	// HRI escaped as content type considered "Binary" (NUL)
	// check(__LINE__, std::string("12\00034", 5), BarcodeFormat::Code39, "]A4", std::string("12\00034", 5), "31 32 00 33 34", false,
	// 	  std::string("]A4\\00002612\00034", 15), "5D 41 34 31 32 00 33 34", "12<NUL>34", "Binary", "0x0 102x0 102x49 0x49");

	check(__LINE__, "1234", BarcodeFormat::Code93, "]G0", "1234", "31 32 33 34", false, "]G0\\0000261234", "5D 47 30 31 32 33 34",
		  "1234", "Text", "0x0 72x0 72x39 0x39"); // Check digits removed

	check(__LINE__, "1234", BarcodeFormat::DataBar, "]e0", "0100000000012348", "30 31 30 30 30 30 30 30 30 30 30 31 32 33 34 38",
		  false, "]e0\\0000260100000000012348", "5D 65 30 30 31 30 30 30 30 30 30 30 30 30 31 32 33 34 38", "(01)00000000012348",
		  "GS1", "1x0 96x0 96x32 1x32");

	check(__LINE__, "1234", {BarcodeFormat::DataBar, "stacked"}, "]e0", "0100000000012348",
		  "30 31 30 30 30 30 30 30 30 30 30 31 32 33 34 38", false, "]e0\\0000260100000000012348",
		  "5D 65 30 30 31 30 30 30 30 30 30 30 30 30 31 32 33 34 38", "(01)00000000012348", "GS1", "1x0 50x0 50x68 1x68");

	check(__LINE__, "[01]12345678901231[20]12[90]123[91]1234", BarcodeFormat::DataBarExpanded, "]e0",
		  "0112345678901231201290123\x1D"
		  "911234",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32 39 30 31 32 33 1D 39 31 31 32 33 34", false,
		  "]e0\\0000260112345678901231201290123\x1D"
		  "911234",
		  "5D 65 30 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32 39 30 31 32 33 1D 39 31 31 32 33 34",
		  "(01)12345678901231(20)12(90)123(91)1234", "GS1", "2x0 246x0 246x33 2x33");

	check(__LINE__, "[01]12345678901231[20]12[90]123[91]1234", {BarcodeFormat::DataBarExpanded, "stacked"}, "]e0",
		  "0112345678901231201290123\x1D"
		  "911234",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32 39 30 31 32 33 1D 39 31 31 32 33 34", false,
		  "]e0\\0000260112345678901231201290123\x1D"
		  "911234",
		  "5D 65 30 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32 39 30 31 32 33 1D 39 31 31 32 33 34",
		  "(01)12345678901231(20)12(90)123(91)1234", "GS1", "0x0 102x0 102x108 0x108");

	// Note not marked as GS1, and hence HRI AI not parenthesized TODO: check
	check(__LINE__, "1234", BarcodeFormat::DataBarLimited, "]e0", "0100000000012348",
		  "30 31 30 30 30 30 30 30 30 30 30 31 32 33 34 38", false, "]e0\\0000260100000000012348",
		  "5D 65 30 30 31 30 30 30 30 30 30 30 30 30 31 32 33 34 38", "(01)00000000012348", "GS1", "1x0 73x0 73x9 1x9");

	check(__LINE__, "1234", BarcodeFormat::DataMatrix, "]d1", "1234", "31 32 33 34", false, "]d4\\0000261234", "5D 64 34 31 32 33 34",
		  "1234", "Text", "0x0 9x0 9x9 0x9", "" /*ecLevel*/, "1" /*version*/);

	// check(__LINE__, "1234", BarcodeFormat::DotCode, "]J0", "1234", "31 32 33 34", false, "]J3\\0000261234",
	// 	  "5D 4A 33 31 32 33 34", "1234", "Text", "1x1 25x1 25x19 1x19", "" /*ecLevel*/, "" /*version*/, 3 /*dataMask*/);

	// DX number only
	check(__LINE__, "77-2", BarcodeFormat::DXFilmEdge, "" /*si*/, "77-2", "37 37 2D 32", false, "\\00002677-2", "37 37 2D 32", "77-2",
		  "Text", "0x0 22x0 22x5 0x5");

	// DX number + frame number
	check(__LINE__, "77-2/62A", BarcodeFormat::DXFilmEdge, "" /*si*/, "77-2/62A", "37 37 2D 32 2F 36 32 41", false, "\\00002677-2/62A",
		  "37 37 2D 32 2F 36 32 41", "77-2/62A", "Text", "0x0 30x0 30x5 0x5");

	check(__LINE__, "123456", BarcodeFormat::EAN8, "]E4", "01234565", "30 31 32 33 34 35 36 35", false, "]E4\\00002601234565",
		  "5D 45 34 30 31 32 33 34 35 36 35", "01234565", "Text", "0x0 66x0 66x59 0x59");

	check(__LINE__, "1234567890128", BarcodeFormat::EAN13, "]E0", "1234567890128", "31 32 33 34 35 36 37 38 39 30 31 32 38", false,
		  "]E0\\0000261234567890128", "5D 45 30 31 32 33 34 35 36 37 38 39 30 31 32 38", "1234567890128", "Text",
		  "0x0 94x0 94x73 0x73");

	// check(__LINE__, "1234", BarcodeFormat::HanXin, "]h0", "1234", "31 32 33 34", false, "]h1\\0000261234",
	// 	  "5D 68 31 31 32 33 34", "1234", "Text", "0x0 22x0 22x22 0x22", "L4", "1" /*version*/);

	check(__LINE__, "1234", BarcodeFormat::ITF, "]I0", "1234", "31 32 33 34", false, "]I0\\0000261234", "5D 49 30 31 32 33 34", "1234",
		  "Text", "0x0 44x0 44x49 0x49");

	check(__LINE__, "1234", BarcodeFormat::MaxiCode, "]U0", "1234", "31 32 33 34", false, "]U2\\0000261234", "5D 55 32 31 32 33 34",
		  "1234", "Text", "0x0 148x0 148x132 0x132", "4" /*ecLevel*/);

	// check(__LINE__, "1234", BarcodeFormat::MicroPDF417, "]L2", "1234", "31 32 33 34", false, "]L1\\0000261234",
	// 	  "5D 4C 31 31 32 33 34", "1234", "Text", "0x0 37x0 37x21 0x21", "64%" /*ecLevel*/);

	check(__LINE__, "1234", BarcodeFormat::MicroQRCode, "]Q1", "1234", "31 32 33 34", false, "]Q2\\0000261234", "5D 51 32 31 32 33 34",
		  "1234", "Text", "0x0 10x0 10x10 0x10", "L", "1" /*version*/);

	check(__LINE__, "1234", BarcodeFormat::PDF417, "]L2", "1234", "31 32 33 34", false, "]L1\\0000261234", "5D 4C 31 31 32 33 34",
		  "1234", "Text", "0x0 102x0 102x17 0x17", "66%" /*ecLevel*/);

	check(__LINE__, "1234", BarcodeFormat::QRCode, "]Q1", "1234", "31 32 33 34", false, "]Q2\\0000261234", "5D 51 32 31 32 33 34",
		  "1234", "Text", "0x0 20x0 20x20 0x20", "H", "1" /*version*/);

	check(__LINE__, "1234", BarcodeFormat::RMQRCode, "]Q1", "1234", "31 32 33 34", false, "]Q2\\0000261234", "5D 51 32 31 32 33 34",
		  "1234", "Text", "0x0 26x0 26x10 0x10", "H", "11" /*version*/);

	check(__LINE__, "1234", BarcodeFormat::UPCA, "]E0", "0000000012348", "30 30 30 30 30 30 30 30 31 32 33 34 38", false,
		  "]E0\\0000260000000012348", "5D 45 30 30 30 30 30 30 30 30 30 31 32 33 34 38", "0000000012348", "Text", "0x0 94x0 94x73 0x73");

	check(__LINE__, "1234", BarcodeFormat::UPCE, "]E0", "0000120000034", "30 30 30 30 31 32 30 30 30 30 30 33 34", false, "]E0\\0000260000120000034",
		  "5D 45 30 30 30 30 30 31 32 30 30 30 30 30 33 34", "0000120000034", "Text", "0x0 50x0 50x73 0x73");
}

TEST(WriteBarcodeTest, ZintISO8859_1)
{
	// Control chars (SOH & DEL)
	check(__LINE__, "12\001\17734", BarcodeFormat::Code128, "]C0", "12\001\17734", "31 32 01 7F 33 34", false,
		  "]C0\\00002612\001\17734", "5D 43 30 31 32 01 7F 33 34", "12<SOH><DEL>34", "Binary");

	// NUL
	check(__LINE__, std::string("12\00034", 5), BarcodeFormat::Code128, "]C0", std::string("12\00034", 5), "31 32 00 33 34", false,
		  std::string("]C0\\00002612\00034", 15), "5D 43 30 31 32 00 33 34", "12<NUL>34", "Binary");

	// Latin-1 (Extended ASCII)
	check(__LINE__, "12é34", BarcodeFormat::Code128, "]C0", "12é34", "31 32 E9 33 34", false, "]C0\\00002612é34",
		  "5D 43 30 31 32 E9 33 34", "12é34", "Text");

	// Control char & Latin-1
	check(__LINE__, "\007Ç", BarcodeFormat::Code128, "]C0", "\007Ç", "07 C7", false, "]C0\\000026\007Ç", "5D 43 30 07 C7", "<BEL>Ç",
		  "Binary");

	// No ECI
	check(__LINE__, "1234é", BarcodeFormat::Aztec, "]z0", "1234é", "31 32 33 34 E9", false, "]z3\\0000261234é",
		  "5D 7A 33 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "35%" /*ecLevel*/, "1" /*version*/);

	// With ECI cOpts.eci(ECI::ISO8859_1);
	// check(__LINE__, "1234é", BarcodeFormat::Aztec, "]z3", "1234é", "31 32 33 34 E9", true, "]z3\\0000261234é",
	// 	  "5D 7A 33 5C 30 30 30 30 30 33 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "17%", "1");

	// No ECI
	check(__LINE__, "1234é", BarcodeFormat::DataMatrix, "]d1", "1234é", "31 32 33 34 E9", false, "]d4\\0000261234é",
		  "5D 64 34 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "" /*ecLevel*/, "2" /*version*/);

	// With ECI cOpts.eci(ECI::ISO8859_1);
	// check(__LINE__, "1234é", BarcodeFormat::DataMatrix, "]d4", "1234é", "31 32 33 34 E9", true,
	// 	  "]d4\\0000261234é", "5D 64 34 5C 30 30 30 30 30 33 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "" /*ecLevel*/, "3");

	// No ECIMaxiCode
	check(__LINE__, "1234é", BarcodeFormat::MaxiCode, "]U0", "1234é", "31 32 33 34 E9", false, "]U2\\0000261234é",
		  "5D 55 32 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "4" /*ecLevel*/);

	// With ECI cOpts.eci(ECI::ISO8859_1);
	// check(__LINE__, "1234é", BarcodeFormat::MaxiCode, "]U2", "1234é", "31 32 33 34 E9", true,
	// 	  "]U2\\0000261234é", "5D 55 32 5C 30 30 30 30 30 33 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "4" /*ecLevel*/);

	// No ECI
	check(__LINE__, "1234é", BarcodeFormat::PDF417, "]L2", "1234é", "31 32 33 34 E9", false, "]L1\\0000261234é",
		  "5D 4C 31 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "57%");

	// With ECI cOpts.eci(ECI::ISO8859_1);
	// check(__LINE__, "1234é", BarcodeFormat::PDF417, "]L1", "1234é", "31 32 33 34 E9", true, "]L1\\0000261234é",
	// 	  "5D 4C 31 5C 30 30 30 30 30 33 31 32 33 34 E9", "1234é", "Text", "" /*position*/, "50%");
	//

	// No ECI
	check(__LINE__, "1234é", BarcodeFormat::QRCode, "]Q1", "1234é", "31 32 33 34 E9", false, "]Q2\\0000261234é",
		  "5D 51 32 31 32 33 34 E9", "1234é", "Text", "0x0 20x0 20x20 0x20", "H", "1");

	// With ECI	cOpts.eci(ECI::ISO8859_1);
	// check(__LINE__, "1234é", BarcodeFormat::QRCode, "]Q2", "1234é", "31 32 33 34 E9", true, "]Q2\\0000261234é",
	// 	  "5D 51 32 5C 30 30 30 30 30 33 31 32 33 34 E9", "1234é", "Text", "0x0 20x0 20x20 0x20", "H", "1");

	// No ECI
	check(__LINE__, "1234é", BarcodeFormat::RMQRCode, "]Q1", "1234é", "31 32 33 34 E9", false, "]Q2\\0000261234é",
		  "5D 51 32 31 32 33 34 E9", "1234é", "Text", "0x0 26x0 26x10 0x10", "H", "11");

	// With ECI cOpts.eci(ECI::ISO8859_1);
	// check(__LINE__, "1234é", BarcodeFormat::RMQRCode, "]Q2", "1234é", "31 32 33 34 E9", true,
	// 	  "]Q2\\0000261234é", "5D 51 32 5C 30 30 30 30 30 33 31 32 33 34 E9", "1234é", "Text", "0x0 26x0 26x10 0x10", "M", "11");
}

TEST(WriteBarcodeTest, ZintGS1)
{
	check(__LINE__, "(01)12345678901231(20)12", {BarcodeFormat::Aztec, "GS1"}, "]z1", "01123456789012312012",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", false, "]z4\\00002601123456789012312012",
		  "5D 7A 34 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", "(01)12345678901231(20)12", "GS1",
		  "0x0 19x0 19x19 0x19", "50%", "2" /*version*/);

	check(__LINE__, "(01)12345678901231(20)12", {BarcodeFormat::Code128, "GS1"}, "]C1", "01123456789012312012",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", false, "]C1\\00002601123456789012312012",
		  "5D 43 31 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", "(01)12345678901231(20)12", "GS1",
		  "0x0 155x0 155x63 0x63");

	// check(
	// 	__LINE__,
	// 	"(01)04912345123459[15]970331[30]128[10]ABC123", {BarcodeFormat::Code16K, "GS1"},
	// 	"]K1",
	// 	"01049123451234591597033130128\x1D"
	// 	"10ABC123",
	// 	"30 31 30 34 39 31 32 33 34 35 31 32 33 34 35 39 31 35 39 37 30 33 33 31 33 30 31 32 38 1D 31 30 41 42 43 31 32 33", false,
	// 	"]K1\\00002601049123451234591597033130128\x1D"
	// 	"10ABC123",
	// 	"5D 4B 31 30 31 30 34 39 31 32 33 34 35 31 32 33 34 35 39 31 35 39 37 30 33 33 31 33 30 31 32 38 1D 31 30 41 42 43 "
	// 	"31 32 33",
	// 	"(01)04912345123459(15)970331(30)128(10)ABC123", "GS1", "0x0 69x0 69x67 0x67");

	check(__LINE__, "(01)12345678901231(20)12", {BarcodeFormat::DataMatrix, "GS1"}, "]d2", "01123456789012312012",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", false, "]d5\\00002601123456789012312012",
		  "5D 64 35 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", "(01)12345678901231(20)12", "GS1",
		  "0x0 15x0 15x15 0x15", "" /*ecLevel*/, "4" /*version*/);

	// check(__LINE__, "(01)00012345678905(17)201231(10)ABC123456", {BarcodeFormat::DotCode, "GS1"}, "]J1",
	// 	  "01000123456789051720123110ABC123456",
	// 	  "30 31 30 30 30 31 32 33 34 35 36 37 38 39 30 35 31 37 32 30 31 32 33 31 31 30 41 42 43 31 32 33 34 35 36", false,
	// 	  "]J4\\00002601000123456789051720123110ABC123456",
	// 	  "5D 4A 34 30 31 30 30 30 31 32 33 34 35 36 37 38 39 30 35 31 37 32 30 31 32 33 31 31 30 41 42 43 31 32 33 34 35 36",
	// 	  "(01)00012345678905(17)201231(10)ABC123456", "GS1", "1x1 57x1 57x39 1x39", "" /*ecLevel*/, "" /*version*/, 1 /*dataMask*/);

	check(__LINE__, "(01)12345678901231(20)12", {BarcodeFormat::QRCode, "GS1"}, "]Q3", "01123456789012312012",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", false, "]Q4\\00002601123456789012312012",
		  "5D 51 34 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", "(01)12345678901231(20)12", "GS1",
		  "0x0 20x0 20x20 0x20", "Q", "1");

	check(__LINE__, "(01)12345678901231(20)12", {BarcodeFormat::RMQRCode, "GS1"}, "]Q3", "01123456789012312012",
		  "30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", false, "]Q4\\00002601123456789012312012",
		  "5D 51 34 30 31 31 32 33 34 35 36 37 38 39 30 31 32 33 31 32 30 31 32", "(01)12345678901231(20)12", "GS1",
		  "0x0 26x0 26x12 0x12", "M", "17");
}

#endif // #if defined(ZXING_EXPERIMENTAL_API) && defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
