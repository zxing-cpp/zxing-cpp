/*
* Copyright 2017 Axel Waggershauser
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
#include "datamatrix/DMWriter.h"
#include "datamatrix/DMDecoder.h"
#include "datamatrix/DMEncoderContext.h"
#include "DecoderResult.h"
#include "DecodeStatus.h"
#include "BitMatrix.h"
#include "BitMatrixUtility.h"
#include "PseudoRandom.h"
#include "ZXContainerAlgorithms.h"

#include "fstream"

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

	void TestEncodeDecode(const std::wstring& data, DataMatrix::SymbolShape shape = DataMatrix::SymbolShape::NONE)
	{
		DataMatrix::Writer writer;
		writer.setShapeHint(shape);
		BitMatrix matrix = writer.encode(data, 0, 0);
		ASSERT_EQ(matrix.empty(), false);

		DecoderResult res = DataMatrix::Decoder::Decode(matrix);
#ifndef NDEBUG
		if (!res.isValid() || data != res.text())
			Utility::WriteBitMatrixAsPBM(matrix, std::ofstream("failed-datamatrix.pbm"), 4);
#endif
		ASSERT_EQ(res.isValid(), true) << "text size: " << data.size() << ", code size: " << matrix.height() << "x"
									   << matrix.width() << ", shape: " << static_cast<int>(shape) << "\n"
									   << (matrix.width() < 80 ? Utility::ToString(matrix) : std::string());
		EXPECT_EQ(data, res.text()) << "text size: " << data.size() << ", code size: " << matrix.height() << "x"
									<< matrix.width() << ", shape: " << static_cast<int>(shape) << "\n"
									<< (matrix.width() < 80 ? Utility::ToString(matrix) : std::string());
	}
}

TEST(DMEncodeDecodeTest, EncodeDecodeSquare)
{
	std::wstring text[] = {
	    L"Abc123!",
	    L"Lorem ipsum. http://test/",
	    L"AAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAAN",
	    L"http://test/~!@#*^%&)__ ;:'\"[]{}\\|-+-=`1029384",
	    L"http://test/~!@#*^%&)__ ;:'\"[]{}\\|-+-=`1029384756<>/?abc"
		"Four score and seven our forefathers brought forth",
	    L"In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam"
	    " cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum"
		" est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue"
		" auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla"
		" ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id"
	    " elementum sapien dolor et diam.",
	    L"In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam"
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
		" erat pulvinar nisi, id elementum sapien dolor et diam.",
	};

	for (auto data : text)
		TestEncodeDecode(data, DataMatrix::SymbolShape::SQUARE);
}

TEST(DMEncodeDecodeTest, EncodeDecodeRectangle)
{
	std::wstring text[] = {
	    L"Abc123!",
	    L"Lorem ipsum. http://test/",
	    L"3i0QnD^RcZO[\\#!]1,9zIJ{1z3qrvsq",
	    L"AAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAAN",
	    L"http://test/~!@#*^%&)__ ;:'\"[]{}\\|-+-=`1029384",
	    L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};

	for (auto data : text)
		for (size_t len	= 1; len <= data.size(); ++len)
			TestEncodeDecode(data.substr(0, len), DataMatrix::SymbolShape::RECTANGLE);
}

TEST(DMEncodeDecodeTest, EDIFACTWithEOD)
{
	using namespace DataMatrix;
	std::wstring text[] = {
	    L"https://test~[******]_",
		L"abc<->ABCDE",
		L"<ABCDEFG><ABCDEFGK>",
		L"*CH/GN1/022/00",
	};
	for (auto data : text)
		for (auto shape : {SymbolShape::NONE, SymbolShape::SQUARE, SymbolShape::RECTANGLE})
			TestEncodeDecode(data, shape);
}

