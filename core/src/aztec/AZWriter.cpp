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

#include "aztec/AZWriter.h"
#include "aztec/AZEncoder.h"
#include "CharacterSet.h"
#include "TextEncoder.h"

#include <algorithm>

namespace ZXing {
namespace Aztec {

Writer::Writer() :
	_encoding(CharacterSet::ISO8859_1),
	_eccPercent(Encoder::DEFAULT_EC_PERCENT),
	_layers(Encoder::DEFAULT_AZTEC_LAYERS)
{
}

BitMatrix
Writer::encode(const std::wstring& contents, int width, int height) const
{
	std::string bytes = TextEncoder::FromUnicode(contents, _encoding);
	EncodeResult aztec = Encoder::Encode(bytes, _eccPercent, _layers);
	// Minimum required quite zone for Aztec is 0
	return Inflate(std::move(aztec.matrix), width, height, 0);
}

} // Aztec
} // ZXing
