/*
* Copyright 2017 Axel Waggershauser
* Copyright 2013 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrixIO.h"
#include "DecoderResult.h"
#include "datamatrix/DMDecoder.h"
#include "datamatrix/DMWriter.h"

#include "gtest/gtest.h"

using namespace ZXing;

namespace {

	void TestEncodeDecode(const std::wstring& data, DataMatrix::SymbolShape shape = DataMatrix::SymbolShape::NONE)
	{
		BitMatrix matrix = DataMatrix::Writer().setMargin(0).setShapeHint(shape).encode(data, 0, 0);
		ASSERT_EQ(matrix.empty(), false);

		DecoderResult res = DataMatrix::Decode(matrix);
#ifdef PRINT_DEBUG
		if (!res.isValid() || data != res.text())
			SaveAsPBM(matrix, "failed-datamatrix.pbm", 4);
#endif
		ASSERT_EQ(res.isValid(), true) << "text size: " << data.size() << ", code size: " << matrix.height() << "x"
									   << matrix.width() << ", shape: " << static_cast<int>(shape) << "\n"
									   << (matrix.width() < 80 ? ToString(matrix) : std::string());
		EXPECT_EQ(data, res.text()) << "text size: " << data.size() << ", code size: " << matrix.height() << "x"
									<< matrix.width() << ", shape: " << static_cast<int>(shape) << "\n"
									<< (matrix.width() < 80 ? ToString(matrix) : std::string());
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

	for (auto& data : text)
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

	for (auto& data : text)
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
	for (auto& data : text)
		for (auto shape : {SymbolShape::NONE, SymbolShape::SQUARE, SymbolShape::RECTANGLE})
			TestEncodeDecode(data, shape);
}

