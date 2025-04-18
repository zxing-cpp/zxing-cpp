/*
 * Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Content.h"

#include "CharacterSet.h"
#include "ECI.h"
#include "HRI.h"
#include "TextDecoder.h"
#include "Utf.h"
#include "ZXAlgorithms.h"

#if !defined(ZXING_READERS) && !defined(ZXING_WRITERS)
#include "Version.h"
#endif

#include <cctype>

namespace ZXing {

std::string ToString(ContentType type)
{
	const char* t2s[] = {"Text", "Binary", "Mixed", "GS1", "ISO15434", "UnknownECI"};
	return t2s[static_cast<int>(type)];
}

template <typename FUNC>
void Content::ForEachECIBlock(FUNC func) const
{
	ECI defaultECI = hasECI ? ECI::ISO8859_1 : ECI::Unknown;
	if (encodings.empty())
		func(defaultECI, 0, Size(bytes));
	else if (encodings.front().pos != 0)
		func(defaultECI, 0, encodings.front().pos);

	for (int i = 0; i < Size(encodings); ++i) {
		auto [eci, start] = encodings[i];
		int end = i + 1 == Size(encodings) ? Size(bytes) : encodings[i + 1].pos;

		if (start != end)
			func(eci, start, end);
	}
}

void Content::switchEncoding(ECI eci, bool isECI)
{
	// remove all non-ECI entries on first ECI entry
	if (isECI && !hasECI)
		encodings.clear();
	if (isECI || !hasECI)
		encodings.push_back({eci, Size(bytes)});

	hasECI |= isECI;
}

Content::Content() {}

Content::Content(ByteArray&& bytes, SymbologyIdentifier si) : bytes(std::move(bytes)), symbology(si) {}

void Content::switchEncoding(CharacterSet cs)
{
	switchEncoding(ToECI(cs), false);
}

void Content::append(const Content& other)
{
	if (!hasECI && other.hasECI)
		encodings.clear();
	if (other.hasECI || !hasECI)
		for (auto& e : other.encodings)
			encodings.push_back({e.eci, Size(bytes) + e.pos});
	append(other.bytes);

	hasECI |= other.hasECI;
}

void Content::erase(int pos, int n)
{
	bytes.erase(bytes.begin() + pos, bytes.begin() + pos + n);
	for (auto& e : encodings)
		if (e.pos > pos)
			e.pos -= n;
}

void Content::insert(int pos, std::string_view str)
{
	bytes.insert(bytes.begin() + pos, str.begin(), str.end());
	for (auto& e : encodings)
		if (e.pos > pos)
			e.pos += Size(str);
}

bool Content::canProcess() const
{
	return std::all_of(encodings.begin(), encodings.end(), [](Encoding e) { return CanProcess(e.eci); });
}

std::string Content::render(bool withECI) const
{
	if (empty() || !canProcess())
		return {};

#ifdef ZXING_READERS
	std::string res;
	res.reserve(bytes.size() * 2);
	if (withECI)
		res += symbology.toString(true);
	ECI lastECI = ECI::Unknown;
	auto fallbackCS = defaultCharset;
	if (!hasECI && fallbackCS == CharacterSet::Unknown)
		fallbackCS = guessEncoding();

	ForEachECIBlock([&](ECI eci, int begin, int end) {
		// basic idea: if IsText(eci), we transcode it to UTF8, otherwise we treat it as binary but
		// transcoded it to valid UTF8 bytes seqences representing the code points 0-255. The eci we report
		// back to the caller by inserting their "\XXXXXX" ECI designator is UTF8 for text and
		// the original ECI for everything else.
		// first determine how to decode the content (use fallback if unknown)
		auto inEci = IsText(eci) ? eci : eci == ECI::Unknown ? ToECI(fallbackCS) : ECI::Binary;
		if (withECI) {
			// then find the eci to report back in the ECI designator
			auto outEci = IsText(inEci) ? ECI::UTF8 : eci;

			if (lastECI != outEci)
				res += ToString(outEci);
			lastECI = outEci;

			for (auto c : BytesToUtf8(bytes.asView(begin, end - begin), inEci)) {
				res += c;
				if (c == '\\') // in the ECI protocol a '\' (0x5c) has to be doubled, works only because 0x5c can only mean `\`
					res += c;
			}
		} else {
			res += BytesToUtf8(bytes.asView(begin, end - begin), inEci);
		}
	});

	return res;
#else
	//TODO: replace by proper construction from encoded data from within zint
	return std::string(bytes.asString());
#endif
}

std::string Content::text(TextMode mode) const
{
	switch (mode) {
	case TextMode::Plain: return render(false);
	case TextMode::ECI: return render(true);
	case TextMode::HRI:
		switch (type()) {
#ifdef ZXING_READERS
		case ContentType::GS1: {
			auto plain = render(false);
			auto hri = HRIFromGS1(plain);
			return hri.empty() ? plain : hri;
		}
		case ContentType::ISO15434: return HRIFromISO15434(render(false));
		case ContentType::Text: return render(false);
#endif
		default: return text(TextMode::Escaped);
		}
	case TextMode::Hex: return ToHex(bytes);
	case TextMode::Escaped: return EscapeNonGraphical(render(false));
	}

	return {}; // silence compiler warning
}

std::wstring Content::utfW() const
{
	return FromUtf8(render(false));
}

ByteArray Content::bytesECI() const
{
	if (empty())
		return {};

	ByteArray res;
	res.reserve(3 + bytes.size() + hasECI * encodings.size() * 7);

	// report ECI protocol only if actually found ECI data in the barode bit stream
	// see also https://github.com/zxing-cpp/zxing-cpp/issues/936
	res.append(symbology.toString(hasECI));

	if (hasECI)
		ForEachECIBlock([&](ECI eci, int begin, int end) {
			if (hasECI)
				res.append(ToString(eci));

			for (auto b : bytes.asView(begin, end - begin)) {
				res.push_back(b);
				if (b == '\\') // in the ECI protocol a '\' has to be doubled
					res.push_back(b);
			}
		});
	else
		res.append(bytes);

	return res;
}

CharacterSet Content::guessEncoding() const
{
#ifdef ZXING_READERS
	// assemble all blocks with unknown encoding
	ByteArray input;
	ForEachECIBlock([&](ECI eci, int begin, int end) {
		if (eci == ECI::Unknown)
			input.append(bytes.asView(begin, end - begin));
	});

	if (input.empty())
		return CharacterSet::Unknown;

	return GuessTextEncoding(input);
#else
	return CharacterSet::ISO8859_1;
#endif
}

ContentType Content::type() const
{
#if 1 //def ZXING_READERS
	if (empty())
		return ContentType::Text;

	if (!canProcess())
		return ContentType::UnknownECI;

	if (symbology.aiFlag == AIFlag::GS1)
		return ContentType::GS1;

	// check for the absolut minimum of a ISO 15434 conforming message ("[)>" + RS + digit + digit)
	if (bytes.size() > 6 && bytes.asString(0, 4) == "[)>\x1E" && std::isdigit(bytes[4]) && std::isdigit(bytes[5]))
		return ContentType::ISO15434;

	ECI fallback = ToECI(guessEncoding());
	std::vector<bool> binaryECIs;
	ForEachECIBlock([&](ECI eci, int begin, int end) {
		if (eci == ECI::Unknown)
			eci = fallback;
		binaryECIs.push_back((!IsText(eci)
							  || (ToInt(eci) > 0 && ToInt(eci) < 28 && ToInt(eci) != 25
								  && std::any_of(bytes.begin() + begin, bytes.begin() + end,
												 [](auto c) { return c < 0x20 && c != 0x9 && c != 0xa && c != 0xd; }))));
	});

	if (!Contains(binaryECIs, true))
		return ContentType::Text;
	if (!Contains(binaryECIs, false))
		return ContentType::Binary;

	return ContentType::Mixed;
#else
	//TODO: replace by proper construction from encoded data from within zint
	return ContentType::Text;
#endif
}

} // namespace ZXing
