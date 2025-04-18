/*
* Copyright 2016 Nu-book Inc.
* Copyright 2022 gitlost
* Copyright 2025 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "TextDecoder.h"

#include "ZXAlgorithms.h"
#include "libzueci/zueci.h"

#include <cassert>
#include <stdexcept>

namespace ZXing {

std::string BytesToUtf8(ByteView bytes, ECI eci)
{
	constexpr unsigned int replacement = 0xFFFD;
	constexpr unsigned int flags = ZUECI_FLAG_SB_STRAIGHT_THRU | ZUECI_FLAG_SJIS_STRAIGHT_THRU;
	int utf8_len;

	if (eci == ECI::Unknown)
		eci = ECI::Binary;

	int error_number = zueci_dest_len_utf8(ToInt(eci), bytes.data(), bytes.size(), replacement, flags, &utf8_len);
	if (error_number >= ZUECI_ERROR)
		throw std::runtime_error("zueci_dest_len_utf8 failed");

	std::string utf8(utf8_len, 0);

	error_number = zueci_eci_to_utf8(ToInt(eci), bytes.data(), bytes.size(), replacement, flags,
									 reinterpret_cast<uint8_t*>(utf8.data()), &utf8_len);
	if (error_number >= ZUECI_ERROR)
		throw std::runtime_error("zueci_eci_to_utf8 failed");

	assert(Size(utf8) == utf8_len);

	return utf8;
}

/**
* @param bytes bytes encoding a string, whose encoding should be guessed
* @return name of guessed encoding; at the moment will only guess one of:
*  {@link #SHIFT_JIS}, {@link #UTF8}, {@link #ISO88591}, or the platform
*  default encoding if none of these can possibly be correct
*/
CharacterSet GuessTextEncoding(ByteView bytes, CharacterSet fallback)
{
	// For now, merely tries to distinguish ISO-8859-1, UTF-8 and Shift_JIS,
	// which should be by far the most common encodings.
	bool canBeISO88591 = true;
	bool canBeShiftJIS = true;
	bool canBeUTF8 = true;
	int utf8BytesLeft = 0;
	//int utf8LowChars = 0;
	int utf2BytesChars = 0;
	int utf3BytesChars = 0;
	int utf4BytesChars = 0;
	int sjisBytesLeft = 0;
	//int sjisLowChars = 0;
	int sjisKatakanaChars = 0;
	//int sjisDoubleBytesChars = 0;
	int sjisCurKatakanaWordLength = 0;
	int sjisCurDoubleBytesWordLength = 0;
	int sjisMaxKatakanaWordLength = 0;
	int sjisMaxDoubleBytesWordLength = 0;
	//int isoLowChars = 0;
	//int isoHighChars = 0;
	int isoHighOther = 0;

	bool utf8bom = bytes.size() > 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF;

	for (int value : bytes)
	{
		if(!(canBeISO88591 || canBeShiftJIS || canBeUTF8))
			break;

		// UTF-8 stuff
		if (canBeUTF8) {
			if (utf8BytesLeft > 0) {
				if ((value & 0x80) == 0) {
					canBeUTF8 = false;
				}
				else {
					utf8BytesLeft--;
				}
			}
			else if ((value & 0x80) != 0) {
				if ((value & 0x40) == 0) {
					canBeUTF8 = false;
				}
				else {
					utf8BytesLeft++;
					if ((value & 0x20) == 0) {
						utf2BytesChars++;
					}
					else {
						utf8BytesLeft++;
						if ((value & 0x10) == 0) {
							utf3BytesChars++;
						}
						else {
							utf8BytesLeft++;
							if ((value & 0x08) == 0) {
								utf4BytesChars++;
							}
							else {
								canBeUTF8 = false;
							}
						}
					}
				}
			} //else {
			  //utf8LowChars++;
			  //}
		}

		// ISO-8859-1 stuff
		if (canBeISO88591) {
			if (value > 0x7F && value < 0xA0) {
				canBeISO88591 = false;
			}
			else if (value > 0x9F) {
				if (value < 0xC0 || value == 0xD7 || value == 0xF7) {
					isoHighOther++;
				} //else {
				  //isoHighChars++;
				  //}
			} //else {
			  //isoLowChars++;
			  //}
		}

		// Shift_JIS stuff
		if (canBeShiftJIS) {
			if (sjisBytesLeft > 0) {
				if (value < 0x40 || value == 0x7F || value > 0xFC) {
					canBeShiftJIS = false;
				}
				else {
					sjisBytesLeft--;
				}
			}
			else if (value == 0x80 || value == 0xA0 || value > 0xEF) {
				canBeShiftJIS = false;
			}
			else if (value < 0x20 && value != 0xa && value != 0xd) {
				canBeShiftJIS = false; // use non-printable ASCII as indication for binary content
			}
			else if (value > 0xA0 && value < 0xE0) {
				sjisKatakanaChars++;
				sjisCurDoubleBytesWordLength = 0;
				sjisCurKatakanaWordLength++;
				if (sjisCurKatakanaWordLength > sjisMaxKatakanaWordLength) {
					sjisMaxKatakanaWordLength = sjisCurKatakanaWordLength;
				}
			}
			else if (value > 0x7F) {
				sjisBytesLeft++;
				//sjisDoubleBytesChars++;
				sjisCurKatakanaWordLength = 0;
				sjisCurDoubleBytesWordLength++;
				if (sjisCurDoubleBytesWordLength > sjisMaxDoubleBytesWordLength) {
					sjisMaxDoubleBytesWordLength = sjisCurDoubleBytesWordLength;
				}
			}
			else {
				//sjisLowChars++;
				sjisCurKatakanaWordLength = 0;
				sjisCurDoubleBytesWordLength = 0;
			}
		}
	}

	if (canBeUTF8 && utf8BytesLeft > 0) {
		canBeUTF8 = false;
	}
	if (canBeShiftJIS && sjisBytesLeft > 0) {
		canBeShiftJIS = false;
	}

	// Easy -- if there is BOM or at least 1 valid not-single byte character (and no evidence it can't be UTF-8), done
	if (canBeUTF8 && (utf8bom || utf2BytesChars + utf3BytesChars + utf4BytesChars > 0)) {
		return CharacterSet::UTF8;
	}

	bool assumeShiftJIS = fallback == CharacterSet::Shift_JIS || fallback == CharacterSet::EUC_JP;
	// Easy -- if assuming Shift_JIS or at least 3 valid consecutive not-ascii characters (and no evidence it can't be), done
	if (canBeShiftJIS && (assumeShiftJIS || sjisMaxKatakanaWordLength >= 3 || sjisMaxDoubleBytesWordLength >= 3)) {
		return CharacterSet::Shift_JIS;
	}
	// Distinguishing Shift_JIS and ISO-8859-1 can be a little tough for short words. The crude heuristic is:
	// - If we saw
	//   - only two consecutive katakana chars in the whole text, or
	//   - at least 10% of bytes that could be "upper" not-alphanumeric Latin1,
	// - then we conclude Shift_JIS, else ISO-8859-1
	if (canBeISO88591 && canBeShiftJIS) {
		return (sjisMaxKatakanaWordLength == 2 && sjisKatakanaChars == 2) || isoHighOther * 10 >= Size(bytes)
			? CharacterSet::Shift_JIS : CharacterSet::ISO8859_1;
	}

	// Otherwise, try in order ISO-8859-1, Shift JIS, UTF-8 and fall back to default platform encoding
	if (canBeISO88591) {
		return CharacterSet::ISO8859_1;
	}
	if (canBeShiftJIS) {
		return CharacterSet::Shift_JIS;
	}
	if (canBeUTF8) {
		return CharacterSet::UTF8;
	}
	// Otherwise, we take a wild guess with platform encoding
	return fallback;
}

} // ZXing
