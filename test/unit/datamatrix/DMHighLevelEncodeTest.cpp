/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2006 Jeremias Maerki.
*/
// SPDX-License-Identifier: Apache-2.0

#include "ByteArray.h"
#include "CharacterSet.h"
#include "ZXAlgorithms.h"
#include "datamatrix/DMHighLevelEncoder.h"
#include "datamatrix/DMSymbolInfo.h"

#include "gtest/gtest.h"

namespace ZXing {
	namespace DataMatrix {
		void OverrideSymbolSet(const SymbolInfo* symbols, size_t count);
		void UseDefaultSymbolSet();
	}
}

using namespace ZXing;


  //private static void useTestSymbols() {
  //  SymbolInfo.overrideSymbolSet(TEST_SYMBOLS);
  //}

  //private static void resetSymbols() {
  //  SymbolInfo.overrideSymbolSet(SymbolInfo.PROD_SYMBOLS);
  //}

namespace {

	static DataMatrix::SymbolInfo TEST_SYMBOLS[] = {
		{ false, 3, 5, 8, 8, 1 },
		{ false, 5, 7, 10, 10, 1 },
		{ true, 5, 7, 16, 6, 1 },
		{ false, 8, 10, 12, 12, 1 },
		{ true, 10, 11, 14, 6, 2 },
		{ false, 13, 0, 0, 0, 1 },
		{ false, 77, 0, 0, 0, 1 },
		//The last entries are fake entries to test special conditions with C40 encoding
	};

	/**
	* Convert a string of char codewords into a different string which lists each character
	* using its decimal value.
	*
	* @param codewords the codewords
	* @return the visualized codewords
	*/
	static std::string Visualize(const ByteArray& codewords) {
		std::stringstream buf;
		for (int i = 0; i < Size(codewords); i++) {
			if (i > 0) {
				buf << ' ';
			}
			buf << (int)codewords[i];
		}
		return buf.str();
	}

	std::string Encode(const std::wstring& text) {
		return Visualize(DataMatrix::Encode(text));
	}

	std::wstring CreateBinaryMessage(int len) {
		std::wstring buf;
		buf.append(L"\xAB\xE4\xF6\xFC\xE9\xE0\xE1-");
		for (int i = 0; i < len - 9; i++) {
			buf.push_back(L'\xB7');
		}
		buf.push_back(L'\xBB');
		return buf;
	}
}

TEST(DMHighLevelEncodeTest, ASCIIEncodation)
{
	std::string visualized = Encode(L"123456");
	EXPECT_EQ(visualized, "142 164 186");

	visualized = Encode(L"123456\xA3");
	EXPECT_EQ(visualized, "142 164 186 235 36");

	visualized = Encode(L"30Q324343430794<OQQ");
	EXPECT_EQ(visualized, "160 82 162 173 173 173 137 224 61 80 82 82");
}


TEST(DMHighLevelEncodeTest, C40EncodationBasic1)
{
	std::string visualized = Encode(L"AIMAIMAIM");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 254");
	//230 shifts to C40 encodation, 254 unlatches, "else" case
}

TEST(DMHighLevelEncodeTest, C40EncodationBasic2)
{
	std::string visualized = Encode(L"AIMAIAB");
	EXPECT_EQ(visualized, "230 91 11 90 255 254 67 129");
	//"B" is normally encoded as "15" (one C40 value)
	//"else" case: "B" is encoded as ASCII

	visualized = Encode(L"AIMAIAb");
	EXPECT_EQ(visualized, "66 74 78 66 74 66 99 129"); //Encoded as ASCII
	//Alternative solution:
	//EXPECT_EQ(visualized, "230 91 11 90 255 254 99 129", visualized);
	//"b" is normally encoded as "Shift 3, 2" (two C40 values)
	//"else" case: "b" is encoded as ASCII

	visualized = Encode(L"AIMAIMAIM\xCB");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 254 235 76");
	//Alternative solution:
	//EXPECT_EQ(visualized, "230 91 11 91 11 91 11 11 9 254", visualized);
	//Expl: 230 = shift to C40, "91 11" = "AIM",
	//"11 9" = "�" = "Shift 2, UpperShift, <char>
	//"else" case

	visualized = Encode(L"AIMAIMAIM\xEB");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 254 235 108"); //Activate when additional rectangulars are available
	//Expl: 230 = shift to C40, "91 11" = "AIM",
	//"�" in C40 encodes to: 1 30 2 11 which doesn't fit into a triplet
	//"10 243" =
	//254 = unlatch, 235 = Upper Shift, 108 = � = 0xEB/235 - 128 + 1
	//"else" case
}

TEST(DMHighLevelEncodeTest, C40EncodationSpecExample)
{
	//Example in Figure 1 in the spec
	std::string visualized = Encode(L"A1B2C3D4E5F6G7H8I9J0K1L2");
	EXPECT_EQ(visualized, "230 88 88 40 8 107 147 59 67 126 206 78 126 144 121 35 47 254");
}

TEST(DMHighLevelEncodeTest, C40EncodationSpecialCases1)
{
	//Special tests avoiding ultra-long test strings because these tests are only used
	//with the 16x48 symbol (47 data codewords)
	DataMatrix::OverrideSymbolSet(TEST_SYMBOLS, Size(TEST_SYMBOLS));

	std::string visualized = Encode(L"AIMAIMAIMAIMAIMAIM");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 91 11 91 11 91 11");
	//case "a": Unlatch is not required

	visualized = Encode(L"AIMAIMAIMAIMAIMAI");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 91 11 91 11 90 241");
	// case "b": Add trailing shift 0 and Unlatch is not required

	visualized = Encode(L"AIMAIMAIMAIMAIMA");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 91 11 91 11 254 66");
	// case "c": Unlatch and write last character in ASCII

	DataMatrix::UseDefaultSymbolSet();

	visualized = Encode(L"AIMAIMAIMAIMAIMAI");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 91 11 91 11 254 66 74 129 237");

	visualized = Encode(L"AIMAIMAIMA");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 66");
	// case "d": Skip Unlatch and write last character in ASCII
}

TEST(DMHighLevelEncodeTest, C40EncodationSpecialCases2) {

	std::string visualized = Encode(L"AIMAIMAIMAIMAIMAIMAI");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 91 11 91 11 91 11 254 66 74");
	//available > 2, rest = 2 --> unlatch and encode as ASCII
}

TEST(DMHighLevelEncodeTest, TextEncodation)
{
	std::string visualized = Encode(L"aimaimaim");
	EXPECT_EQ(visualized, "239 91 11 91 11 91 11 254");
	//239 shifts to Text encodation, 254 unlatches

	visualized = Encode(L"aimaimaim'");
	EXPECT_EQ(visualized, "239 91 11 91 11 91 11 254 40 129");
	// EXPECT_EQ(visualized, "239 91 11 91 11 91 11 7 49 254");
	// This is an alternative, but doesn't strictly follow the rules in the spec.

	visualized = Encode(L"aimaimaIm");
	EXPECT_EQ(visualized, "239 91 11 91 11 87 218 110");

	visualized = Encode(L"aimaimaimB");
	EXPECT_EQ(visualized, "239 91 11 91 11 91 11 254 67 129");

	visualized = Encode(L"aimaimaim{txt}\x04");
	EXPECT_EQ(visualized, "239 91 11 91 11 91 11 16 218 236 107 181 69 254 129 237");
}

TEST(DMHighLevelEncodeTest, X12Encodation)
{
	// 238 shifts to X12 encodation, 254 unlatches
	std::string visualized = Encode(L"ABC>ABC123>AB");
	EXPECT_EQ(visualized, "238 89 233 14 192 100 207 44 31 67");

	visualized = Encode(L"ABC>ABC123>ABC");
	EXPECT_EQ(visualized, "238 89 233 14 192 100 207 44 31 254 67 68");

	visualized = Encode(L"ABC>ABC123>ABCD");
	EXPECT_EQ(visualized, "238 89 233 14 192 100 207 44 31 96 82 254");

	visualized = Encode(L"ABC>ABC123>ABCDE");
	EXPECT_EQ(visualized, "238 89 233 14 192 100 207 44 31 96 82 70");

	visualized = Encode(L"ABC>ABC123>ABCDEF");
	EXPECT_EQ(visualized, "238 89 233 14 192 100 207 44 31 96 82 254 70 71 129 237");
}

TEST(DMHighLevelEncodeTest, EDIFACTEncodation)
{
	// 240 shifts to EDIFACT encodation
	std::string visualized = Encode(L".A.C1.3.DATA.123DATA.123DATA");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 16 21 1 187 28 179 16 21 1 187 28 179 16 21 1");

	visualized = Encode(L".A.C1.3.X.X2..");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 98 230 50 47 47");

	visualized = Encode(L".A.C1.3.X.X2.");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 98 230 50 47 129");

	visualized = Encode(L".A.C1.3.X.X2");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 98 230 50");

	visualized = Encode(L".A.C1.3.X.X");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 98 230 31");

	visualized = Encode(L".A.C1.3.X.");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 98 231 192");

	visualized = Encode(L".A.C1.3.X");
	EXPECT_EQ(visualized, "240 184 27 131 198 236 238 89");

	// Checking temporary unlatch from EDIFACT
	visualized = Encode(L".XXX.XXX.XXX.XXX.XXX.XXX.\xFCXX.XXX.XXX.XXX.XXX.XXX.XXX");
	EXPECT_EQ(visualized, "240 185 134 24 185 134 24 185 134 24 185 134 24 185 134 24 185 134 24"
						  " 124 47 235 125 240" // <- this is the temporary unlatch
						  " 97 139 152 97 139 152 97 139 152 97 139 152 97 139 152 97 139 152 89 89");
}

TEST(DMHighLevelEncodeTest, Base256Encodation)
{
	// 231 shifts to Base256 encodation
	std::string visualized = Encode(L"\xAB\xE4\xF6\xFC\xE9\xBB");
	EXPECT_EQ(visualized, "231 44 108 59 226 126 1 104");
	visualized = Encode(L"\xAB\xE4\xF6\xFC\xE9\xE0\xBB");
	EXPECT_EQ(visualized, "231 51 108 59 226 126 1 141 254 129");
	visualized = Encode(L"\xAB\xE4\xF6\xFC\xE9\xE0\xE1\xBB");
	EXPECT_EQ(visualized, "231 44 108 59 226 126 1 141 36 147");

	visualized = Encode(L" 23\xA3"); // ASCII only (for reference)
	EXPECT_EQ(visualized, "33 153 235 36 129");

	visualized = Encode(L"\xAB\xE4\xF6\xFC\xE9\xBB 234"); // Mixed Base256 + ASCII
	EXPECT_EQ(visualized, "231 51 108 59 226 126 1 104 99 153 53 129");

	visualized = Encode(L"\xAB\xE4\xF6\xFC\xE9\xBB 23\xA3 1234567890123456789");
	EXPECT_EQ(visualized, "231 55 108 59 226 126 1 104 99 10 161 167 185 142 164 186 208"
						  " 220 142 164 186 208 58 129 59 209 104 254 150 45");

	visualized = Encode(CreateBinaryMessage(20));
	EXPECT_EQ(visualized, "231 44 108 59 226 126 1 141 36 5 37 187 80 230 123 17 166 60 210 103 253 150");
	visualized = Encode(CreateBinaryMessage(19)); // padding necessary at the end
	EXPECT_EQ(visualized, "231 63 108 59 226 126 1 141 36 5 37 187 80 230 123 17 166 60 210 103 1 129");

	visualized = Encode(CreateBinaryMessage(276));
	std::string expectedStart = "231 38 219 2 208 120 20 150 35";
	std::string epxectedEnd = "146 40 194 129";
	EXPECT_EQ(visualized.substr(0, expectedStart.length()), expectedStart);
	EXPECT_EQ(visualized.substr(visualized.length() - epxectedEnd.length()), epxectedEnd);

	visualized = Encode(CreateBinaryMessage(277));
	expectedStart = "231 38 220 2 208 120 20 150 35";
	epxectedEnd = "146 40 190 87";
	EXPECT_EQ(visualized.substr(0, expectedStart.length()), expectedStart);
	EXPECT_EQ(visualized.substr(visualized.length() - epxectedEnd.length()), epxectedEnd);
}

TEST(DMHighLevelEncodeTest, UnlatchingFromC40)
{
	std::string visualized = Encode(L"AIMAIMAIMAIMaimaimaim");
	EXPECT_EQ(visualized, "230 91 11 91 11 91 11 254 66 74 78 239 91 11 91 11 91 11");
}

TEST(DMHighLevelEncodeTest, UnlatchingFromText)
{
	std::string visualized = Encode(L"aimaimaimaim12345678");
	EXPECT_EQ(visualized, "239 91 11 91 11 91 11 91 11 254 142 164 186 208 129 237");
}

TEST(DMHighLevelEncodeTest, tHelloWorld)
{
	std::string visualized = Encode(L"Hello World!");
	EXPECT_EQ(visualized, "73 239 116 130 175 123 148 64 158 233 254 34");
}

TEST(DMHighLevelEncodeTest, Bug1664266)
{
	// There was an exception and the encoder did not handle the unlatching from
	// EDIFACT encoding correctly

	std::string visualized = Encode(L"CREX-TAN:h");
	EXPECT_EQ(visualized, "240 13 33 88 181 64 78 124 59 105");

	visualized = Encode(L"CREX-TAN:hh");
	EXPECT_EQ(visualized, "240 13 33 88 181 64 78 124 59 105 105 129");

	visualized = Encode(L"CREX-TAN:hhh");
	EXPECT_EQ(visualized, "240 13 33 88 181 64 78 124 59 105 105 105");
}

TEST(DMHighLevelEncodeTest, X12Unlatch)
{
	std::string visualized = Encode(L"*DTCP01");
	EXPECT_EQ(visualized, "238 9 10 104 141 254 50 129");
}

TEST(DMHighLevelEncodeTest, X12Unlatch2)
{
	std::string visualized = Encode(L"*DTCP0");
	EXPECT_EQ(visualized, "238 9 10 104 141");
}

TEST(DMHighLevelEncodeTest, Bug3048549)
{
	// There was an IllegalArgumentException for an illegal character here because
	// of an encoding problem of the character 0x0060 in Java source code.
	std::string visualized = Encode(L"fiykmj*Rh2`,e6");
	EXPECT_EQ(visualized, "239 122 87 154 40 7 171 115 207 12 130 71 155 254 129 237");
}

TEST(DMHighLevelEncodeTest, MacroCharacters)
{
	std::string visualized = Encode(L"[)>\x1E""05\x1D""5555\x1C""6666\x1E\x04");
	//EXPECT_EQ(visualized, "92 42 63 31 135 30 185 185 29 196 196 31 5 129 87 237");
	EXPECT_EQ(visualized, "236 185 185 29 196 196 129 56");
}

TEST(DMHighLevelEncodeTest, EncodingWithStartAsX12AndLatchToEDIFACTInTheMiddle)
{
	std::string visualized = Encode(L"*MEMANT-1F-MESTECH");
	EXPECT_EQ(visualized, "238 10 99 164 204 254 240 82 220 70 180 209 83 80 80 200");
}

TEST(DMHighLevelEncodeTest, EDIFACTWithEODBug)
{
	std::string visualized = Visualize(
		DataMatrix::Encode(L"abc<->ABCDE", CharacterSet::ISO8859_1, DataMatrix::SymbolShape::SQUARE, -1, -1, -1, -1));
	// switch to EDIFACT on '<', uses 10 code words + 2 padding. Buggy code introduced invalid 254 after the 5
	EXPECT_EQ(visualized, "98 99 100 240 242 223 129 8 49 5 129 147");
}

//  @Ignore
//  @Test  
//  public void testDataURL() {
//
//    byte[] data = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
//        0x7E, 0x7F, (byte) 0x80, (byte) 0x81, (byte) 0x82};
//    String expected = encodeHighLevel(new String(data, StandardCharsets.ISO_8859_1));
//    String visualized = encodeHighLevel("url(data:text/plain;charset=iso-8859-1,"
//                                            + "%00%01%02%03%04%05%06%07%08%09%0A%7E%7F%80%81%82)");
//    assertEquals(expected, visualized);
//    assertEquals("1 2 3 4 5 6 7 8 9 10 11 231 153 173 67 218 112 7", visualized);
//
//    visualized = encodeHighLevel("url(data:;base64,flRlc3R+)");
//    assertEquals("127 85 102 116 117 127 129 56", visualized);
//  }
//
//  private static String encodeHighLevel(String msg) {
//    CharSequence encoded = HighLevelEncoder.encodeHighLevel(msg);
//    //DecodeHighLevel.decode(encoded);
//    return visualize(encoded);
//  }
//  
//  /**
//   * Convert a string of char codewords into a different string which lists each character
//   * using its decimal value.
//   *
//   * @param codewords the codewords
//   * @return the visualized codewords
//   */
//  static String visualize(CharSequence codewords) {
//    StringBuilder sb = new StringBuilder();
//    for (int i = 0; i < codewords.length(); i++) {
//      if (i > 0) {
//        sb.append(' ');
//      }
//      sb.append((int) codewords.charAt(i));
//    }
//    return sb.toString();
//  }
//
//}
