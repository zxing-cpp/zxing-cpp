/*
 * Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Content.h"

#include "ByteArray.h"
#include "CharacterSet.h"
#include "ECI.h"
#include "HRI.h"
#include "TextDecoder.h"
#include "Utf.h"
#include "ZXAlgorithms.h"
#include "Version.h"

#include <cassert>

namespace ZXing {

std::string ToString(ContentType type)
{
	return EnumToString(type, {"Text", "Binary", "Mixed", "GS1", "ISO15434", "UnknownECI"});
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

Content::Content(ByteArray&& bytes, SymbologyIdentifier si, CharacterSet defaultCharset)
	: bytes(std::move(bytes)), symbology(si), defaultCharset(defaultCharset) {}

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
#if !ZXING_ENABLE_UNICODE
	bool hasNonUtf8 = false;
#endif
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
		// transcoded it to valid UTF8 bytes sequences representing the code points 0-255. The eci we report
		// back to the caller by inserting their "\XXXXXX" ECI designator is UTF8 for text and
		// the original ECI for everything else.
		// first determine how to decode the content (use fallback if unknown)
		auto inEci = IsText(eci) ? eci : eci == ECI::Unknown ? ToECI(fallbackCS) : ECI::Binary;
		auto utf8 = BytesToUtf8(bytes.asView(begin, end - begin), inEci);
#if !ZXING_ENABLE_UNICODE
		hasNonUtf8 |= utf8.empty();
#endif
		if (withECI) {
			// then find the eci to report back in the ECI designator
			auto outEci = IsText(inEci) ? ECI::UTF8 : eci;

			if (lastECI != outEci)
				res += ToString(outEci);
			lastECI = outEci;

			for (auto c : utf8) {
				res += c;
				if (c == '\\') // in the ECI protocol a '\' (0x5c) has to be doubled, works only because 0x5c can only mean `\`
					res += c;
			}
		} else {
			res += utf8;
		}
	});
#if !ZXING_ENABLE_UNICODE
	if (hasNonUtf8)
		return {};
#endif
	return res;
#elif defined(ZXING_USE_ZINT)
	assert(!utf8Cache.empty());
	if (!withECI)
		return std::accumulate(utf8Cache.begin(), utf8Cache.end(), std::string());

	std::string res;
	res.reserve(3 + TransformReduce(utf8Cache, 0, std::size<std::string>) * 2 + encodings.size() * 7);
	res += symbology.toString(true);

	ECI lastECI = ECI::Unknown;
	auto fallbackCS = defaultCharset;
	if (!hasECI && fallbackCS == CharacterSet::Unknown)
		fallbackCS = guessEncoding();

	assert(utf8Cache.size() == encodings.size());
	for (int i = 0; i < Size(encodings); ++i) {
		const auto eci = encodings[i].eci;
		const auto inEci = IsText(eci) ? eci : eci == ECI::Unknown ? ToECI(fallbackCS) : ECI::Binary;
		const auto outEci = IsText(inEci) ? ECI::UTF8 : eci;

		if (lastECI != outEci)
			res += ToString(outEci);
		lastECI = outEci;

		for (auto c : utf8Cache[i]) {
			res += c;
			if (c == '\\') // in the ECI protocol a '\' (0x5c) has to be doubled, works only because 0x5c can only mean `\`
				res += c;
		}
	}

	return res;
#else
	(void)withECI;
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
#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
		case ContentType::GS1: {
			auto plain = render(false);
			auto hri = HRIFromGS1(plain);
			return hri.empty() ? EscapeNonGraphical(plain) : hri;
		}
		case ContentType::ISO15434: return HRIFromISO15434(render(false));
		case ContentType::Text: return render(false);
#endif
		default: return text(TextMode::Escaped);
		}
	case TextMode::Escaped: return EscapeNonGraphical(render(false));
	case TextMode::Hex: return ToHex(bytes);
	case TextMode::HexECI: return ToHex(bytesECI());
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

#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
/**
* @param bytes bytes encoding a string, whose encoding should be guessed
* @param fallback encoding to use if none of the supported encodings can be determined
* @return name of guessed encoding; at the moment will only guess one of:
*  {@link #SHIFT_JIS}, {@link #UTF8}, {@link #ISO88591}, or the specified
*  fallback if none of these can possibly be correct
*/
CharacterSet GuessTextEncoding(ByteView bytes, CharacterSet fallback = CharacterSet::ISO8859_1)
{
	// ISO-8859-1 is the default for ambiguous byte-mode text (including QR data without ECI).
	bool canBeISO88591 = true;
	bool canBeShiftJIS = true;
	bool canBeUTF8 = IsValidUtf8(bytes);
	bool hasHighByte = false;
	bool hasShiftJISC1Lead = false;
	int sjisKatakanaRun = 0;
	int sjisDoubleByteRun = 0;
	int sjisMaxKatakanaWordLength = 0;
	int sjisMaxDoubleBytesWordLength = 0;

	for (int i = 0; i < Size(bytes); ++i) {
		const int value = bytes[i];
		hasHighByte |= value > 0x7F;
		// C1 (0x80-0x9F) bytes are unlikely Latin-1 text and more likely a Shift_JIS lead.
		canBeISO88591 &= value < 0x80 || value > 0x9F;

		if (!canBeShiftJIS)
			continue;

		// Evidence for Shift_JIS: a byte in 0x81-0x9F (the lead bytes of most double-byte characters),
		// or a run of 3 half-width katakana or of 3 double-byte characters.
		// Shift_JIS singles are 0x00-0x7F or 0xA1-0xDF; leads 0x81-0x9F / 0xE0-0xEF
		// require trails 0x40-0x7E / 0x80-0xFC. Exclude 0xF0-0xFC leads, reserved for JIS X 0208
		// user-defined characters, but continue to allow them as trails. Other C0 controls are binary evidence.
		if (value <= 0x7F) {
			if (value < 0x20 && value != 0x0A && value != 0x0D)
				canBeShiftJIS = false;
			sjisKatakanaRun = sjisDoubleByteRun = 0;
		} else if (value >= 0xA1 && value <= 0xDF) {
			sjisDoubleByteRun = 0;
			if (++sjisKatakanaRun > sjisMaxKatakanaWordLength)
				sjisMaxKatakanaWordLength = sjisKatakanaRun;
		} else if ((value >= 0x81 && value <= 0x9F) || (value >= 0xE0 && value <= 0xEF)) {
			if (++i == Size(bytes) || !((bytes[i] >= 0x40 && bytes[i] <= 0x7E) || (bytes[i] >= 0x80 && bytes[i] <= 0xFC))) {
				canBeShiftJIS = false;
				continue;
			}
			hasShiftJISC1Lead |= value <= 0x9F;
			sjisKatakanaRun = 0;
			if (++sjisDoubleByteRun > sjisMaxDoubleBytesWordLength)
				sjisMaxDoubleBytesWordLength = sjisDoubleByteRun;
		} else {
			canBeShiftJIS = false;
		}
	}

	// Valid non-ASCII UTF-8 is decisive; ASCII alone is valid in every encoding.
	if (canBeUTF8 && hasHighByte)
		return CharacterSet::UTF8;

	bool assumeShiftJIS = fallback == CharacterSet::Shift_JIS || fallback == CharacterSet::EUC_JP;
	// Unless the fallback favors Japanese, require a C1 lead or a run of three kana or double-byte characters.
	if (canBeShiftJIS && (assumeShiftJIS || hasShiftJISC1Lead || sjisMaxKatakanaWordLength >= 3 || sjisMaxDoubleBytesWordLength >= 3))
		return CharacterSet::Shift_JIS;

	// Prefer the QR Code default when both ISO-8859-1 and Shift_JIS are plausible.
	if (canBeISO88591)
		return CharacterSet::ISO8859_1;

	if (canBeShiftJIS)
		return CharacterSet::Shift_JIS;

	if (canBeUTF8)
		return CharacterSet::UTF8;

	// If none of the supported encodings fit, use the caller-provided fallback.
	return fallback;
}
#endif

CharacterSet Content::guessEncoding() const
{
#if defined(ZXING_READERS) || defined(ZXING_USE_ZINT)
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

	// check for the absolute minimum of a ISO 15434 conforming message ("[)>" + RS + digit + digit)
	if (bytes.size() > 6 && bytes.asString(0, 4) == "[)>\x1E" && IsDigit(bytes[4]) && IsDigit(bytes[5]))
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
