/*
* Copyright 2016 ZXing authors
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

#include "pdf417/PDFCodewordDecoder.h"
#include "pdf417/PDFCommon.h"

#include <vector>
#include <numeric>

namespace ZXing {
namespace Pdf417 {

typedef std::array<std::array<float, Common::BARS_IN_MODULE>, Common::SYMBOL_COUNT> RatioTableType;
typedef std::array<int, Common::BARS_IN_MODULE> ModuleBitCountType;

static const RatioTableType& GetRatioTable()
{
	auto initTable = [](RatioTableType& table) -> RatioTableType& {
		for (int i = 0; i < Common::SYMBOL_COUNT; i++) {
			int currentSymbol = Common::SYMBOL_TABLE[i];
			int currentBit = currentSymbol & 0x1;
			for (int j = 0; j < Common::BARS_IN_MODULE; j++) {
				float size = 0.0f;
				while ((currentSymbol & 0x1) == currentBit) {
					size += 1.0f;
					currentSymbol >>= 1;
				}
				currentBit = currentSymbol & 0x1;
				table[i][Common::BARS_IN_MODULE - j - 1] = size / Common::MODULES_IN_CODEWORD;
			}
		}
		return table;
	};

	static RatioTableType table;
	static const auto& ref = initTable(table);
	return ref;
}

static void SampleBitCounts(const ModuleBitCountType& moduleBitCount, ModuleBitCountType& result)
{
	float bitCountSum = static_cast<float>(std::accumulate(moduleBitCount.begin(), moduleBitCount.end(), 0));
	std::fill(result.begin(), result.end(), 0);
	int bitCountIndex = 0;
	int sumPreviousBits = 0;
	for (int i = 0; i < Common::MODULES_IN_CODEWORD; i++) {
		float sampleIndex = bitCountSum / (2 * Common::MODULES_IN_CODEWORD) + (i * bitCountSum) / Common::MODULES_IN_CODEWORD;
		if (sumPreviousBits + moduleBitCount[bitCountIndex] <= sampleIndex) {
			sumPreviousBits += moduleBitCount[bitCountIndex];
			bitCountIndex++;
			if (bitCountIndex == moduleBitCount.size()) { // this check is not done in original code, so I guess this should not happen?
				break;
			}
		}
		result[bitCountIndex]++;
	}
}


static int GetBitValue(const ModuleBitCountType& moduleBitCount)
{
	int result = 0;
	for (size_t i = 0; i < moduleBitCount.size(); i++) {
		for (int bit = 0; bit < moduleBitCount[i]; bit++) {
			result = (result << 1) | (i % 2 == 0 ? 1 : 0);
		}
	}
	return result;
}

static int GetDecodedCodewordValue(const ModuleBitCountType& moduleBitCount)
{
	int decodedValue = GetBitValue(moduleBitCount);
	return Common::GetCodeword(decodedValue) == -1 ? -1 : decodedValue;
}


static int GetClosestDecodedValue(const ModuleBitCountType& moduleBitCount)
{
	static const RatioTableType& ratioTable = GetRatioTable();

	float bitCountSum = (float)std::accumulate(moduleBitCount.begin(), moduleBitCount.end(), 0);
	std::array<float, Common::BARS_IN_MODULE> bitCountRatios;
	for (int i = 0; i < Common::BARS_IN_MODULE; i++) {
		bitCountRatios[i] = moduleBitCount[i] / bitCountSum;
	}
	float bestMatchError = std::numeric_limits<float>::max();
	int bestMatch = -1;
	for (size_t j = 0; j < ratioTable.size(); j++) {
		float error = 0.0f;
		auto& ratioTableRow = ratioTable[j];
		for (int k = 0; k < Common::BARS_IN_MODULE; k++) {
			float diff = ratioTableRow[k] - bitCountRatios[k];
			error += diff * diff;
			if (error >= bestMatchError) {
				break;
			}
		}
		if (error < bestMatchError) {
			bestMatchError = error;
			bestMatch = Common::SYMBOL_TABLE[j];
		}
	}
	return bestMatch;
}

int
CodewordDecoder::GetDecodedValue(const std::array<int, Common::BARS_IN_MODULE>& moduleBitCount)
{
	ModuleBitCountType sampled = {};
	SampleBitCounts(moduleBitCount, sampled);
	int decodedValue = GetDecodedCodewordValue(sampled);
	if (decodedValue != -1) {
		return decodedValue;
	}
	return GetClosestDecodedValue(moduleBitCount);
}

} // Pdf417
} // ZXing
