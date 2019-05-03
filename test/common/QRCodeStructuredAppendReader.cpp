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

#include "QRCodeStructuredAppendReader.h"
#include "HybridBinarizer.h"
#include "TextUtfEncoding.h"
#include "Result.h"
#include "DecodeHints.h"
#include "qrcode/QRReader.h"
#include "ImageLoader.h"

#include <algorithm>

namespace ZXing { namespace Test {

QRCodeStructuredAppendReader::QRCodeStructuredAppendReader(const std::shared_ptr<ImageLoader>& imgLoader)
: _imageLoader(imgLoader)
{
}

TestReader::ReadResult
QRCodeStructuredAppendReader::readMultiple(const std::vector<std::wstring>& filenames, int rotation) const
{
	TestReader::ReadResult result;
	DecodeHints hints;
	QRCode::Reader reader(hints);
	std::list<Result> allResults;
	int prevParity = -1;
	for (const auto& imagePath : filenames) {
		auto image = _imageLoader->load(imagePath);
		ZXing::HybridBinarizer binarizer(image, false);
		auto r = reader.decode(*binarizer.rotated(rotation));
		if (r.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_CODE_COUNT, 0) != filenames.size()) {
			return TestReader::ReadResult();
		}
		auto parity = r.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_PARITY, -1);
		if (prevParity != -1 && prevParity != parity) {
			return TestReader::ReadResult();
		}
		prevParity = parity;
		allResults.push_back(r);
	}

	allResults.sort([](const Result &r1, const Result &r2) {
		auto s1 = r1.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_SEQUENCE, -1);
		auto s2 = r2.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_SEQUENCE, -1);
		return s1 < s2;
	});

	if (!allResults.empty()) {
		result.format = "QR_CODE";
		for (const auto& r : allResults) {
			result.text.append(r.text());
		}
	}
	return result;
}

}} // ZXing::Test
