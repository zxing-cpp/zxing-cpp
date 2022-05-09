/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "QRWriter.h"

#include "BitMatrix.h"
#include "CharacterSet.h"
#include "QREncodeResult.h"
#include "QREncoder.h"
#include "QRErrorCorrectionLevel.h"

#include <stdexcept>
#include <utility>

namespace ZXing::QRCode {

static const int QUIET_ZONE_SIZE = 4;

Writer::Writer()
	: _margin(QUIET_ZONE_SIZE),
	  _ecLevel(ErrorCorrectionLevel::Low),
	  _encoding(CharacterSet::Unknown),
	  _version(0),
	  _useGs1Format(false),
	  _maskPattern(-1)
{}

BitMatrix Writer::encode(const std::wstring& contents, int width, int height) const
{
	if (contents.empty()) {
		throw std::invalid_argument("Found empty contents");
	}

	if (width < 0 || height < 0) {
		throw std::invalid_argument("Requested dimensions are invalid");
	}

	EncodeResult code = Encode(contents, _ecLevel, _encoding, _version, _useGs1Format, _maskPattern);
	return Inflate(std::move(code.matrix), width, height, _margin);
}

} // namespace ZXing::QRCode
