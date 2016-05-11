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

#include "Result.h"

namespace ZXing {

Result::Result(ErrorStatus status) :
	_status(status)
{
}

Result::Result(const std::wstring& text, const ByteArray& rawBytes, const std::vector<ResultPoint>& resultPoints, BarcodeFormat format, time_point tt) :
	_status(ErrorStatus::NoError),
	_text(text),
	_rawBytes(rawBytes),
	_resultPoints(resultPoints),
	_format(format),
	_timestamp(tt)
{
}

void
Result::addResultPoints(const std::vector<ResultPoint>& points)
{
	size_t oldSize = _resultPoints.size();
	_resultPoints.resize(oldSize + points.size());
	std::copy(points.begin(), points.end(), _resultPoints.begin() + oldSize);
}

} // ZXing
