/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2013 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "aztec/AZEncoder.h"
#include "aztec/AZDetectorResult.h"
#include "aztec/AZDecoder.h"
#include "aztec/AZWriter.h"
#include "DecoderResult.h"
#include "DecodeStatus.h"
#include "BitMatrix.h"
#include "BitMatrixUtility.h"
#include "PseudoRandom.h"
#include "CharacterSet.h"
#include "TextEncoder.h"
#include "TextDecoder.h"

#include <algorithm>

namespace testing {
	namespace internal {
		inline bool operator==(const std::string& a, const std::wstring& b) {
			return a.length() == b.length() && std::equal(a.begin(), a.end(), b.begin());
		}
	}
}

#include "gtest/gtest.h"

using namespace ZXing;

namespace {

	void TestEncodeDecode(const std::string& data, bool compact, int layers) {

		Aztec::EncodeResult aztec = Aztec::Encoder::Encode(data, 25, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
		ASSERT_EQ(aztec.compact, compact) << "Unexpected symbol format (compact)";
		ASSERT_EQ(aztec.layers, layers) << "Unexpected nr. of layers";

		DecoderResult res =
			Aztec::Decoder::Decode({aztec.matrix.copy(), {}, aztec.compact, aztec.codeWords, aztec.layers});
		ASSERT_EQ(res.isValid(), true);
		EXPECT_EQ(data, res.text());

		// Check error correction by introducing a few minor errors
		PseudoRandom random(0x12345678);
		BitMatrix matrix = aztec.matrix.copy();
		auto x = random.next(0, matrix.width() - 1);
		auto y = random.next(0, 1);
		matrix.flip(x, y);
		x = random.next(0, matrix.width() - 1);
		y = matrix.height() - 2 + random.next(0, 1);
		matrix.flip(x, y);
		x = random.next(0, 1);
		y = random.next(0, matrix.height() - 1);
		matrix.flip(x, y);
		x = matrix.width() - 2 + random.next(0, 1);
		y = random.next(0, matrix.height() - 1);
		matrix.flip(x, y);

		res = Aztec::Decoder::Decode({std::move(matrix), {}, aztec.compact, aztec.codeWords, aztec.layers});
		ASSERT_EQ(res.isValid(), true);
		EXPECT_EQ(data, res.text());
	}

	void TestWriter(const std::wstring& data, CharacterSet charset, int eccPercent, bool compact, int layers) {
		// 1. Perform an encode-decode round-trip because it can be lossy.
		// 2. Aztec Decoder currently always decodes the data with a LATIN-1 charset:

		std::string textBytes = TextEncoder::FromUnicode(data, charset);

		Aztec::Writer writer;
		writer.setEncoding(charset);
		writer.setEccPercent(eccPercent);
		BitMatrix matrix = writer.encode(data, 0, 0);
		Aztec::EncodeResult aztec = Aztec::Encoder::Encode(textBytes, eccPercent, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
		EXPECT_EQ(aztec.compact, compact) << "Unexpected symbol format (compact)";
		EXPECT_EQ(aztec.layers, layers) << "Unexpected nr. of layers";

		EXPECT_EQ(aztec.matrix, matrix);

		std::wstring expectedData = TextDecoder::ToUnicode(textBytes, CharacterSet::ISO8859_1);
		DecoderResult res = Aztec::Decoder::Decode({matrix.copy(), {}, aztec.compact, aztec.codeWords, aztec.layers});
		EXPECT_EQ(res.isValid(), true);
		EXPECT_EQ(res.text(), expectedData);

		// Check error correction by introducing up to eccPercent/2 errors
		int ecWords = aztec.codeWords * eccPercent / 100 / 2;
		PseudoRandom random(0x12345678);
		for (int i = 0; i < ecWords; i++) {
			// don't touch the core
			int x = random.next(0, 1) == 1 ?
				random.next(0, aztec.layers * 2 - 1)
				: matrix.width() - 1 - random.next(0, aztec.layers * 2 - 1);
			int y = random.next(0, 1) == 1 ?
				random.next(0, aztec.layers * 2 - 1)
				: matrix.height() - 1 - random.next(0, aztec.layers * 2 - 1);
			matrix.flip(x, y);
		}
		res = Aztec::Decoder::Decode({std::move(matrix), {}, aztec.compact, aztec.codeWords, aztec.layers});
		EXPECT_EQ(res.isValid(), true);
		EXPECT_EQ(res.text(), expectedData);
	}
}

TEST(AZEncodeDecodeTest, EncodeDecode1)
{
	TestEncodeDecode("Abc123!", true, 1);
}

TEST(AZEncodeDecodeTest, EncodeDecode2)
{
	TestEncodeDecode("Lorem ipsum. http://test/", true, 2);
}

TEST(AZEncodeDecodeTest, EncodeDecode3)
{
	TestEncodeDecode("AAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAAN", true, 3);
}

TEST(AZEncodeDecodeTest, EncodeDecode4)
{
	TestEncodeDecode("http://test/~!@#*^%&)__ ;:'\"[]{}\\|-+-=`1029384", true, 4);
}

TEST(AZEncodeDecodeTest, EncodeDecode5)
{
	TestEncodeDecode("http://test/~!@#*^%&)__ ;:'\"[]{}\\|-+-=`1029384756<>/?abc"
		"Four score and seven our forefathers brought forth", false, 5);
}

TEST(AZEncodeDecodeTest, EncodeDecode10)
{
	TestEncodeDecode("In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam"
		" cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum"
		" est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue"
		" auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla"
		" ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id"
		" elementum sapien dolor et diam.", false, 10);
}

TEST(AZEncodeDecodeTest, EncodeDecode23)
{
	TestEncodeDecode("In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam"
		" cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum"
		" est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue"
		" auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla"
		" ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id"
		" elementum sapien dolor et diam. Donec ac nunc sodales elit placerat eleifend."
		" Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra fringilla, risus"
		" justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo diam, lobortis eu"
		" tristique ac, p.In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus"
		" quis diam cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec"
		" laoreet rutrum est, nec convallis mauris condimentum sit amet. Phasellus gravida,"
		" justo et congue auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec"
		" lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar"
		" nisi, id elementum sapien dolor et diam. Donec ac nunc sodales elit placerat"
		" eleifend. Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra"
		" fringilla, risus justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo"
		" diam, lobortis eu tristique ac, p. In ut magna vel mauris malesuada dictum. Nulla"
		" ullamcorper metus quis diam cursus facilisis. Sed mollis quam id justo rutrum"
		" sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum sit amet."
		" Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat, eget hendrerit"
		" felis turpis nec lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo"
		" erat pulvinar nisi, id elementum sapien dolor et diam.", false, 23);
}

TEST(AZEncodeDecodeTest, EncodeDecode31)
{
	TestEncodeDecode("In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam"
		" cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum"
		" est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue"
		" auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla"
		" ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id"
		" elementum sapien dolor et diam. Donec ac nunc sodales elit placerat eleifend."
		" Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra fringilla, risus"
		" justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo diam, lobortis eu"
		" tristique ac, p.In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus"
		" quis diam cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec"
		" laoreet rutrum est, nec convallis mauris condimentum sit amet. Phasellus gravida,"
		" justo et congue auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec"
		" lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar"
		" nisi, id elementum sapien dolor et diam. Donec ac nunc sodales elit placerat"
		" eleifend. Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra"
		" fringilla, risus justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo"
		" diam, lobortis eu tristique ac, p. In ut magna vel mauris malesuada dictum. Nulla"
		" ullamcorper metus quis diam cursus facilisis. Sed mollis quam id justo rutrum"
		" sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum sit amet."
		" Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat, eget hendrerit"
		" felis turpis nec lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo"
		" erat pulvinar nisi, id elementum sapien dolor et diam. Donec ac nunc sodales elit"
		" placerat eleifend. Sed ornare luctus ornare. Vestibulum vehicula, massa at"
		" pharetra fringilla, risus justo faucibus erat, nec porttitor nibh tellus sed est."
		" Ut justo diam, lobortis eu tristique ac, p.In ut magna vel mauris malesuada"
		" dictum. Nulla ullamcorper metus quis diam cursus facilisis. Sed mollis quam id"
		" justo rutrum sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum"
		" sit amet. Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat,"
		" eget hendrerit felis turpis nec lorem. Nulla ultrices, elit pellentesque aliquet"
		" laoreet, justo erat pulvinar nisi, id elementum sapien dolor et diam. Donec ac"
		" nunc sodales elit placerat eleifend. Sed ornare luctus ornare. Vestibulum vehicula,"
		" massa at pharetra fringilla, risus justo faucibus erat, nec porttitor nibh tellus"
		" sed est. Ut justo diam, lobortis eu tris. In ut magna vel mauris malesuada dictum."
		" Nulla ullamcorper metus quis diam cursus facilisis. Sed mollis quam id justo rutrum"
		" sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum sit amet."
		" Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat, eget"
		" hendrerit felis turpis nec lorem.", false, 31);
}

TEST(AZEncodeDecodeTest, AztecWriter)
{
	//TestWriter(L"\x20AC 1 sample data.", CharacterSet::ISO8859_1, 25, true, 2);
	//TestWriter(L"\x20AC 1 sample data.", CharacterSet::ISO8859_15, 25, true, 2);
	TestWriter(L"\x20AC 1 sample data.", CharacterSet::UTF8, 25, true, 2);
	TestWriter(L"\x20AC 1 sample data.", CharacterSet::UTF8, 100, true, 3);
	TestWriter(L"\x20AC 1 sample data.", CharacterSet::UTF8, 300, true, 4);
	TestWriter(L"\x20AC 1 sample data.", CharacterSet::UTF8, 500, false, 5);
		
	// Test AztecWriter defaults
	std::wstring data = L"In ut magna vel mauris malesuada";
	Aztec::Writer writer;
	BitMatrix matrix = writer.encode(data, 0, 0);
	Aztec::EncodeResult aztec =
		Aztec::Encoder::Encode(TextEncoder::FromUnicode(data, CharacterSet::ISO8859_1),
							   Aztec::Encoder::DEFAULT_EC_PERCENT, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
	EXPECT_EQ(matrix, aztec.matrix);
}
