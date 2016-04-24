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

#include "DecoderResult.h"

namespace ZXing {

DecoderResult::DecoderResult(ErrorStatus status) :
	_status(status)
{
}

DecoderResult::DecoderResult(const ByteArray& rawBytes, const String& text, std::list<ByteArray>& byteSegments, const std::string& ecLevel) :
	DecoderResult(rawBytes, text, byteSegments, ecLevel, 0, 0)
{
}

DecoderResult::DecoderResult(const ByteArray& rawBytes, const String& text, std::list<ByteArray>& byteSegments, const std::string& ecLevel, int saSequence, int saParity) :
	_status(ErrorStatus::NoError),
	_rawBytes(rawBytes),
	_text(text),
	_byteSegments(byteSegments),
	_ecLevel(ecLevel),
	_structuredAppendSequenceNumber(saSequence),
	_structuredAppendParity(saParity)
{
}

} // ZXing
