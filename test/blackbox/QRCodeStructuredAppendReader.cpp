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

#include "BinaryBitmap.h"
#include "DecodeHints.h"
#include "ImageLoader.h"
#include "Result.h"
#include "TextUtfEncoding.h"
#include "qrcode/QRReader.h"

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
		if (r.symbolCount() != Size(imgPaths))
			return Result(DecodeStatus::FormatError);
		if (prevParity != -1 && prevParity != r.symbolParity())
			return Result(DecodeStatus::FormatError);
		prevParity = r.symbolParity();
		allResults.push_back(r);
	}

	if (allResults.empty())
		return Result(DecodeStatus::NotFound);

	allResults.sort([](const Result& r1, const Result& r2) { return r1.symbolIndex() < r2.symbolIndex(); });

	std::wstring text;
	for (const auto& r : allResults)
		text.append(r.text());

	return {std::move(text), {}, BarcodeFormat::QRCode};
}

} // ZXing::Test
