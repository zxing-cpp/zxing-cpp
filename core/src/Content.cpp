/*
 * Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Content.h"

#include "CharacterSetECI.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"

namespace ZXing {

void Content::switchEncoding(CharacterSet cs, bool isECI)
{
	if (isECI || !hasECI) {
		if (encodings.back().second == Size(binary))
			encodings.back().first = cs; // no point in recording 0 length segments
		else
			encodings.emplace_back(cs, Size(binary));
	}
	hasECI |= isECI;
}

std::wstring Content::text() const
{
	auto fallbackCS = CharacterSetECI::CharsetFromName(hintedCharset.c_str());
	if (!hasECI && fallbackCS == CharacterSet::Unknown)
		fallbackCS = guessEncoding();

	std::wstring wstr;
	for (int i = 0; i < Size(encodings); ++i) {
		auto [cs, start] = encodings[i];
		int end          = i + 1 == Size(encodings) ? Size(binary) : encodings[i + 1].second;

		if (cs == CharacterSet::Unknown)
			cs = fallbackCS;

		TextDecoder::Append(wstr, binary.data() + start, end - start, cs);
	}
	return wstr;
}

CharacterSet Content::guessEncoding() const
{
	// assemble all blocks with unknown encoding
	ByteArray input;
	for (int i = 0; i < Size(encodings); ++i) {
		auto [cs, start] = encodings[i];
		int end          = i + 1 == Size(encodings) ? Size(binary) : encodings[i + 1].second;
		if (cs == CharacterSet::Unknown)
			input.insert(input.end(), binary.begin() + start, binary.begin() + end);
	}

	if (input.empty())
		return CharacterSet::Unknown;

	return TextDecoder::GuessEncoding(input.data(), input.size(), CharacterSet::BINARY);
}

ContentType Content::type() const
{
	auto isBinary = [](Encoding e) { return e.first == CharacterSet::BINARY || e.first == CharacterSet::Unknown; };

	if (hasECI) {
		if (std::none_of(encodings.begin(), encodings.end(), isBinary))
			return ContentType::Text;
		if (std::all_of(encodings.begin(), encodings.end(), isBinary))
			return ContentType::Binary;
	} else {
		if (std::none_of(encodings.begin(), encodings.end(), isBinary))
			return ContentType::Text;
		auto cs = guessEncoding();
		if (cs == CharacterSet::BINARY)
			return ContentType::Binary;
	}

	return ContentType::Mixed;
}

} // namespace ZXing
