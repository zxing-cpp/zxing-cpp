/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "AZWriter.h"

#include "AZEncoder.h"
#include "CharacterSet.h"
#include "TextEncoder.h"

#include <utility>

namespace ZXing::Aztec {

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
	return Inflate(std::move(aztec.matrix), width, height, _margin);
}

} // namespace ZXing::Aztec
