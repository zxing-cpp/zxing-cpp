/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
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

#include "DMDefaultPlacement.h"
#include "DMVersion.h"
#include "BitMatrix.h"
#include "ByteArray.h"
#include "BitArray.h"

namespace ZXing::DataMatrix {

// Extracts the data bits from a BitMatrix that contains alignment patterns.
static BitMatrix ExtractDataBits(const Version& version, const BitMatrix& bits)
{
	BitMatrix res(version.dataWidth(), version.dataHeight());

	for (int y = 0; y < res.height(); ++y)
		for (int x = 0; x < res.width(); ++x) {
			int ix = x + 1 + (x / version.dataBlockWidth) * 2;
			int iy = y + 1 + (y / version.dataBlockHeight) * 2;
			res.set(x, y, bits.get(ix, iy));
		}

	return res;
}

/**
* <p>Reads the bits in the {@link BitMatrix} representing the mapping matrix (No alignment patterns)
* in the correct order in order to reconstitute the codewords bytes contained within the
* Data Matrix Code.</p>
*
* @return bytes encoded within the Data Matrix Code
*/
ByteArray CodewordsFromBitMatrix(const BitMatrix& bits)
{
	const Version* version = VersionForDimensionsOf(bits);
	if (version == nullptr)
		return {};

	BitMatrix dataBits = ExtractDataBits(*version, bits);

	ByteArray result(version->totalCodewords());
	auto codeword = result.begin();

	VisitMatrix(dataBits.height(), dataBits.width(), [&codeword, &dataBits](const BitPosArray& bitPos) {
		// Read the 8 bits of one of the special corner/utah symbols into the current codeword
		*codeword = 0;
		for (auto& p : bitPos)
			AppendBit(*codeword, dataBits.get(p.col, p.row));
		++codeword;
	});

	if (codeword != result.end())
		return {};

	return result;
}

} // namespace ZXing::DataMatrix
