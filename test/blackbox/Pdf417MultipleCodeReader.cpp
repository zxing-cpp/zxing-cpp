/*
* Copyright 2016 Nu-book Inc.
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

#include "Pdf417MultipleCodeReader.h"
#include "BinaryBitmap.h"
#include "TextUtfEncoding.h"
#include "pdf417/PDFReader.h"
#include "pdf417/PDFDecoderResultExtra.h"
#include "ImageLoader.h"

#include <list>
#include <utility>

namespace ZXing::Test {

Result Pdf417MultipleCodeReader::readMultiple(const std::vector<fs::path>& imgPaths, int rotation)
{
	Pdf417::Reader reader;
	std::list<Result> allResults;
	for (const auto& imgPath : imgPaths) {
		auto results = reader.decodeMultiple(*ImageLoader::load(imgPath).rotated(rotation));
		allResults.insert(allResults.end(), results.begin(), results.end());
	}

	if (allResults.empty())
		return Result(DecodeStatus::NotFound);

	allResults.sort([](const Result &r1, const Result &r2) {
		auto m1 = std::dynamic_pointer_cast<Pdf417::DecoderResultExtra>(r1.metadata().getCustomData(ResultMetadata::PDF417_EXTRA_METADATA));
		auto m2 = std::dynamic_pointer_cast<Pdf417::DecoderResultExtra>(r2.metadata().getCustomData(ResultMetadata::PDF417_EXTRA_METADATA));
		return m1->segmentIndex() < m2->segmentIndex();
	});

	std::wstring text;
	for (const auto& r : allResults)
		text.append(r.text());

	return {std::move(text), {}, BarcodeFormat::PDF_417};
}

} // ZXing::Test
