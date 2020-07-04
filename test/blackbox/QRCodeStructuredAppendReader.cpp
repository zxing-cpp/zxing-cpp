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
#include "TextUtfEncoding.h"
#include "BinaryBitmap.h"
#include "Result.h"
#include "DecodeHints.h"
#include "qrcode/QRReader.h"
#include "ImageLoader.h"

#include <list>
#include <string>
#include <utility>

namespace ZXing::Test {

Result QRCodeStructuredAppendReader::readMultiple(const std::vector<fs::path>& imgPaths, int rotation)
{
	QRCode::Reader reader({});
	std::list<Result> allResults;
	int prevParity = -1;
	for (const auto& imgPath : imgPaths) {
		auto r = reader.decode(*ImageLoader::load(imgPath).rotated(rotation));
		if (r.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_CODE_COUNT, 0) != Size(imgPaths))
			return Result(DecodeStatus::FormatError);
		auto parity = r.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_PARITY, -1);
		if (prevParity != -1 && prevParity != parity)
			return Result(DecodeStatus::FormatError);
		prevParity = parity;
		allResults.push_back(r);
	}

	if (allResults.empty())
		return Result(DecodeStatus::NotFound);

	allResults.sort([](const Result &r1, const Result &r2) {
		auto s1 = r1.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_SEQUENCE, -1);
		auto s2 = r2.metadata().getInt(ResultMetadata::STRUCTURED_APPEND_SEQUENCE, -1);
		return s1 < s2;
	});

	std::wstring text;
	for (const auto& r : allResults)
		text.append(r.text());

	return {std::move(text), {}, BarcodeFormat::QR_CODE};
}

} // ZXing::Test
