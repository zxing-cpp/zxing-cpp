/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2006 Jeremias Maerki
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
#include "gtest/gtest.h"
#include "ByteMatrix.h"
#include "datamatrix/DMDefaultPlacement.h"

  //private static final Pattern SPACE = Pattern.compile(" ");

namespace {

	std::vector<int> Unvisualize(const std::string& visualized) {
		std::vector<int> result;
		std::string::size_type prev = 0;
		auto index = visualized.find(' ');
		while (index != std::string::npos) {
			visualized.substr(prev, index - prev);
			result.push_back(std::stoi(visualized.substr(prev, index - prev)));
			prev = index + 1;
			index = visualized.find(' ', prev);
		}
		result.push_back(std::stoi(visualized.substr(prev)));
		return result;
	}

	std::string ToString(const ZXing::ByteMatrix& matrix)
	{
		std::string result;
		result.reserve((matrix.width() + 1) * matrix.height());
		for (int y = 0; y < matrix.height(); ++y) {
			for (int x = 0; x < matrix.width(); ++x) {
				result.push_back(matrix.get(x, y) == 1 ? '1' : '0');
			}
			result.push_back('\n');
		}
		return result;
	}
}

using namespace ZXing::DataMatrix;

TEST(DMPlacementTest, Placement)
{
    auto codewords = Unvisualize("66 74 78 66 74 78 129 56 35 102 192 96 226 100 156 1 107 221"); //"AIMAIM" encoded
    auto matrix = DefaultPlacement::Place(codewords, 12, 12);
    std::string expected =
        "011100001111\n"
        "001010101000\n"
        "010001010100\n"
        "001010100010\n"
        "000111000100\n"
        "011000010100\n"
        "000100001101\n"
        "011000010000\n"
        "001100001101\n"
        "100010010111\n"
        "011101011010\n"
        "001011001010\n";
    EXPECT_EQ(expected, ToString(matrix));
  }
