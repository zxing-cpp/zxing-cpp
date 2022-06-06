/*
 * Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Content.h"

#include "CharacterSet.h"
#include "ECI.h"
#include "TextDecoder.h"
#include "TextUtfEncoding.h"
#include "ZXContainerAlgorithms.h"

namespace ZXing {

std::string ToString(ContentType type)
{
	const char* t2s[] = {"Text", "Binary", "Mixed", "GS1", "ISO15434", "UnknownECI"};
	return t2s[static_cast<int>(type)];
}

template <typename FUNC>
void Content::ForEachECIBlock(FUNC func) const
{
	for (int i = 0; i < Size(encodings); ++i) {
		auto [eci, start] = encodings[i];
		int end = i + 1 == Size(encodings) ? Size(binary) : encodings[i + 1].pos;

		if (start != end)
			func(eci, start, end);
	}
}

void Content::switchEncoding(ECI eci, bool isECI)
{
	// replace all non-ECI entries on first ECI entry with default ECI
	if (isECI && !hasECI)
		encodings = {{ECI::ISO8859_1, 0}};
	if (isECI || !hasECI) {
		if (encodings.back().pos == Size(binary))
			encodings.back().eci = eci; // no point in recording 0 length segments
		else
			encodings.push_back({eci, Size(binary)});
	}
	hasECI |= isECI;
}

Content::Content() : encodings({{ECI::Unknown, 0}}) {}

Content::Content(ByteArray&& binary) : binary(std::move(binary)), encodings{{ECI::ISO8859_1, 0}} {}

void Content::switchEncoding(CharacterSet cs)
{
	switchEncoding(ToECI(cs), false);
}

void Content::erase(int pos, int n)
{
	binary.erase(binary.begin() + pos, binary.begin() + pos + n);
	for (auto& e : encodings)
		if (e.pos > pos)
			pos -= n;
}

void Content::insert(int pos, const std::string& str)
{
	binary.insert(binary.begin() + pos, str.begin(), str.end());
	for (auto& e : encodings)
		if (e.pos > pos)
			pos += Size(str);
}

bool Content::canProcess() const
{
	return std::all_of(encodings.begin(), encodings.end(), [](Encoding e) { return CanProcess(e.eci); });
}

std::wstring Content::text() const
{
	if (!canProcess())
		return {};

	auto fallbackCS = CharacterSetFromString(hintedCharset);
	if (!hasECI && fallbackCS == CharacterSet::Unknown)
		fallbackCS = guessEncoding();

	std::wstring wstr;
	ForEachECIBlock([&](ECI eci, int begin, int end) {
		CharacterSet cs = eci == ECI::Unknown ? fallbackCS : ToCharacterSet(eci);

		TextDecoder::Append(wstr, binary.data() + begin, end - begin, cs);
	});
	return wstr;
}

std::string Content::utf8Protocol() const
{
	if (empty() || !canProcess())
		return {};

	std::wstring res = TextDecoder::FromLatin1(symbology.toString(true));
	ECI lastECI = ECI::Unknown;
	auto fallbackCS = CharacterSetFromString(hintedCharset);
	if (!hasECI && fallbackCS == CharacterSet::Unknown)
		fallbackCS = guessEncoding();

	ForEachECIBlock([&](ECI eci, int begin, int end) {
		// first determine how to decode the content (choose character set)
		//  * eci == ECI::Unknown implies !hasECI and we guess
		//  * if !IsText(eci) the ToCharcterSet(eci) will return Unknown and we decode as binary
		CharacterSet cs = eci == ECI::Unknown ? fallbackCS : ToCharacterSet(eci);

		// then find the eci to report back in the ECI designator
		if (IsText(ToECI(cs))) // everything decoded as text is reported as utf8
			eci = ECI::UTF8;
		else if (eci == ECI::Unknown) // implies !hasECI and fallbackCS is Unknown or Binary
			eci = ECI::Binary;

		if (lastECI != eci)
			TextDecoder::AppendLatin1(res, ToString(eci));
		lastECI = eci;

		std::wstring tmp;
		TextDecoder::Append(tmp, binary.data() + begin, end - begin, cs);
		for (auto c : tmp) {
			res += c;
			if (c == L'\\') // in the ECI protocol a '\' has to be doubled
				res += c;
		}
	});

	return TextUtfEncoding::ToUtf8(res);
}

ByteArray Content::binaryECI() const
{
	if (empty())
		return {};

	std::string res = symbology.toString(true);

	ForEachECIBlock([&](ECI eci, int begin, int end) {
		if (hasECI)
			res += ToString(eci);

		for (int i = begin; i != end; ++i) {
			char c = static_cast<char>(binary[i]);
			res += c;
			if (c == '\\') // in the ECI protocol a '\' has to be doubled
				res += c;
		}
	});

	return ByteArray(res);
}

CharacterSet Content::guessEncoding() const
{
	// assemble all blocks with unknown encoding
	ByteArray input;
	ForEachECIBlock([&](ECI eci, int begin, int end) {
		if (eci == ECI::Unknown)
			input.insert(input.end(), binary.begin() + begin, binary.begin() + end);
	});

	if (input.empty())
		return CharacterSet::Unknown;

	return TextDecoder::GuessEncoding(input.data(), input.size(), CharacterSet::ISO8859_1);
}

ContentType Content::type() const
{
	if (!canProcess())
		return ContentType::UnknownECI;

	if (applicationIndicator == "GS1")
		return ContentType::GS1;

	// check for the absolut minimum of a ISO 15434 conforming message ("[)>" + RS + digit + digit)
	if (binary.size() > 6 && binary.asString(0, 4) == "[)>\x1E" && std::isdigit(binary[4]) && std::isdigit(binary[5]))
		return ContentType::ISO15434;

	ECI fallback = ToECI(guessEncoding());
	std::vector<bool> binaryECIs;
	ForEachECIBlock([&](ECI eci, int begin, int end) {
		if (eci == ECI::Unknown)
			eci = fallback;
		binaryECIs.push_back((!IsText(eci)
							  || (ToInt(eci) > 0 && ToInt(eci) < 28 && ToInt(eci) != 25
								  && std::any_of(binary.begin() + begin, binary.begin() + end,
												 [](auto c) { return c < 0x20 && c != 0xa && c != 0xd; }))));
	});

	if (!Contains(binaryECIs, true))
		return ContentType::Text;
	if (!Contains(binaryECIs, false))
		return ContentType::Binary;

	return ContentType::Mixed;
}

} // namespace ZXing
