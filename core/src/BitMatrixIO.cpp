/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2017 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrixIO.h"

#include "ZXAlgorithms.h"

#include <array>
#include <fstream>

namespace ZXing {

std::string ToSVG(ImageView iv, bool invert)
{
	if (!iv.data())
		return {};

	// see https://stackoverflow.com/questions/10789059/create-qr-code-in-vector-image/60638350#60638350

	// don't use std::ostringstream, see #1151
	std::string res = StrCat(R"(<?xml version="1.0" encoding="UTF-8"?>)", "\n",
							 R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" viewBox="0 0 )", std::to_string(iv.width()),
							 ' ', std::to_string(iv.height()), R"(" stroke="none">)", "\n", "<path d=\"");
	res.reserve(res.size() + iv.width() * iv.height() * 16);

	for (int y = 0; y < iv.height(); ++y)
		for (int x = 0; x < iv.width(); ++x)
			if ((*iv.data(x, y) == 0) ^ invert) {
				res += "M";
				res += std::to_string(x);
				res += ",";
				res += std::to_string(y);
				res += "h1v1h-1z";
			}

	res += "\"/>\n</svg>";

	return res;
}

std::string ToString(const BitMatrix& matrix, char one, char zero, bool addSpace, bool printAsCString)
{
	std::string result;
	result.reserve((addSpace ? 2 : 1) * (matrix.width() * matrix.height()) + matrix.height());
	for (int y = 0; y < matrix.height(); ++y) {
		if (printAsCString)
			result += '"';
		for (auto bit : matrix.row(y)) {
			result += bit ? one : zero;
			if (addSpace)
				result += ' ';
		}
		if (printAsCString)
			result += "\\n\"";
		result += '\n';
	}
	return result;
}

std::string ToString(const BitMatrix& matrix, bool inverted)
{
	constexpr auto map = std::array{" ", "▀", "▄", "█"};
	std::string res;

	for (int y = 0; y < matrix.height(); y += 2) {
		for (int x = 0; x < matrix.width(); ++x) {
			int tp = matrix.get(x, y) ^ inverted;
			int bt = (matrix.height() == 1 && tp) || (y + 1 < matrix.height() && (matrix.get(x, y + 1) ^ inverted));
			res += map[tp | (bt << 1)];
		}
		res.push_back('\n');
	}

	return res;
}

std::string ToSVG(const BitMatrix& matrix, bool invert)
{
	return ToSVG(ImageView(matrix.row(0).begin(), matrix.width(), matrix.height(), ImageFormat::Lum), invert);
}

BitMatrix ParseBitMatrix(const std::string& str, char one, bool expectSpace)
{
	auto lineLength = str.find('\n');
	if (lineLength == std::string::npos)
		return {};

	int strStride = expectSpace ? 2 : 1;
	int height = narrow_cast<int>(str.length() / (lineLength + 1));
	int width = narrow_cast<int>(lineLength / strStride);
	BitMatrix mat(width, height);
	for (int y = 0; y < height; ++y) {
		size_t offset = y * (lineLength + 1);
		for (int x = 0; x < width; ++x, offset += strStride) {
			if (str[offset] == one)
				mat.set(x, y);
		}
	}
	return mat;
}

void SaveAsPBM(const BitMatrix& matrix, const std::string& filename, int quietZone)
{
	auto out = ToMatrix<uint8_t>(Inflate(matrix.copy(), 0, 0, quietZone));
	std::ofstream file(filename);
	file << "P5\n" << out.width() << ' ' << out.height() << "\n255\n";
	file.write(reinterpret_cast<const char*>(out.data()), out.size());
}

} // ZXing
