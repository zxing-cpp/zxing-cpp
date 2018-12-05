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
#include "HybridBinarizer.h"
#include "TextUtfEncoding.h"
#include "Result.h"
#include "pdf417/PDFReader.h"
#include "pdf417/PDFDecoderResultExtra.h"
#include "ImageLoader.h"

#include <algorithm>

namespace ZXing { namespace Test {

Pdf417MultipleCodeReader::Pdf417MultipleCodeReader(const std::shared_ptr<ImageLoader>& imgLoader)
: _imageLoader(imgLoader)
{
}

Pdf417MultipleCodeReader::ReadResult
Pdf417MultipleCodeReader::readMultiple(const std::vector<std::wstring>& filenames, int rotation) const
{
	Pdf417::Reader reader;
	std::list<Result> allResults;
	for (const auto& imagePath : filenames) {
		auto image = _imageLoader->load(imagePath);
		ZXing::HybridBinarizer binarizer(image, false);
		auto results = reader.decodeMultiple(*binarizer.rotated(rotation));
		allResults.insert(allResults.end(), results.begin(), results.end());
	}

	allResults.sort([](const Result &r1, const Result &r2) {
		auto m1 = std::dynamic_pointer_cast<Pdf417::DecoderResultExtra>(r1.metadata().getCustomData(ResultMetadata::PDF417_EXTRA_METADATA));
		auto m2 = std::dynamic_pointer_cast<Pdf417::DecoderResultExtra>(r2.metadata().getCustomData(ResultMetadata::PDF417_EXTRA_METADATA));
		return m1->segmentIndex() < m2->segmentIndex();
	});

	ReadResult result;
	if (!allResults.empty()) {
		result.format = "PDF_417";
		for (const auto& r : allResults) {
			auto txt = r.text();
			result.text.append(txt);
			auto meta = std::dynamic_pointer_cast<Pdf417::DecoderResultExtra>(r.metadata().getCustomData(ResultMetadata::PDF417_EXTRA_METADATA));
			result.fileIds.push_back(meta->fileId());
		}
	}
	return result;
}

}} // ZXing::Test
