#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

#include "ByteArray.h"
#include "BarcodeFormat.h"
#include "ResultPoint.h"
#include "ResultMetadata.h"
#include "DecodeStatus.h"

#include <string>
#include <chrono>
#include <vector>

namespace ZXing {

class ResultMetadata;

/**
* <p>Encapsulates the result of decoding a barcode within an image.</p>
*
* @author Sean Owen
*/
class Result
{
public:
	typedef std::chrono::steady_clock::time_point time_point;

	explicit Result(DecodeStatus status);
	Result(const std::wstring& text, const ByteArray& rawBytes, const std::vector<ResultPoint>& resultPoints, BarcodeFormat format, time_point tt = std::chrono::steady_clock::now());

	bool isValid() const {
		return StatusIsOK(_status);
	}

	DecodeStatus status() const {
		return _status;
	}

	const std::wstring& text() const {
		return _text;
	}

	const ByteArray& rawBytes() const {
		return _rawBytes;
	}

	const std::vector<ResultPoint>& resultPoints() const {
		return _resultPoints;
	}

	void setResultPoints(const std::vector<ResultPoint>& points) {
		_resultPoints = points;
	}

	void addResultPoints(const std::vector<ResultPoint>& points);

	BarcodeFormat format() const {
		return _format;
	}

	time_point timestamp() const {
		return _timestamp;
	}

	const ResultMetadata& metadata() const {
		return _metadata;
	}

	ResultMetadata& metadata() {
		return _metadata;
	}

private:
	DecodeStatus _status;
	std::wstring _text;
	ByteArray _rawBytes;
	std::vector<ResultPoint> _resultPoints;
	BarcodeFormat _format;
	time_point _timestamp;
	ResultMetadata _metadata;
};

} // ZXing
