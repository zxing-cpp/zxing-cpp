#pragma once
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

#include "ZXString.h"
#include "ByteArray.h"
#include "BarcodeFormat.h"
#include "ResultPoint.h"

#include <chrono>
#include <memory>
#include <vector>

namespace ZXing {

class ResultMetadata;

/**
* <p>Encapsulates the result of decoding a barcode within an image.</p>
*/
class Result
{
public:
	typedef std::chrono::steady_clock::time_point time_point;

	Result() {}
	Result(const String& text, const ByteArray& rawBytes, const std::vector<ResultPoint>& resultPoints, BarcodeFormat format, time_point tt = std::chrono::steady_clock::now());
	~Result();

	bool isValid() const {
		return _valid;
	}

	const String& text() const {
		return _text;
	}

	const ByteArray& rawBytes() const {
		return _rawBytes;
	}

	const std::vector<ResultPoint>& resultPoints() const {
		return _resultPoints;
	}

	BarcodeFormat format() const {
		return _format;
	}

	time_point timestamp() const {
		return _timestamp;
	}

	std::shared_ptr<ResultMetadata> metadata() const {
		return _metadata;
	}

	void setMetadata(const std::shared_ptr<ResultMetadata>& m) {
		_metadata = m;
	}

private:
	bool _valid = false;
	String _text;
	ByteArray _rawBytes;
	std::vector<ResultPoint> _resultPoints;
	BarcodeFormat _format;
	time_point _timestamp;
	std::shared_ptr<ResultMetadata> _metadata;
};

} // ZXing
