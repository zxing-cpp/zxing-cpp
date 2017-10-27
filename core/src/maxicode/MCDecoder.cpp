/*
* Copyright 2016 Nu-book Inc.
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

#include "maxicode/MCDecoder.h"
#include "maxicode/MCBitMatrixParser.h"
#include "ByteArray.h"
#include "DecoderResult.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "DecodeStatus.h"
#include "TextDecoder.h"
#include "ZXStrConvWorkaround.h"

#include <array>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ZXing {
namespace MaxiCode {

static const int ALL = 0;
static const int EVEN = 1;
static const int ODD = 2;

static bool CorrectErrors(ByteArray& codewordBytes, int start, int dataCodewords, int ecCodewords, int mode)
{
	int codewords = dataCodewords + ecCodewords;

	// in EVEN or ODD mode only half the codewords
	int divisor = mode == ALL ? 1 : 2;

	// First read into an array of ints
	std::vector<int> codewordsInts(codewords / divisor, 0);
	for (int i = 0; i < codewords; i++) {
		if ((mode == ALL) || (i % 2 == (mode - 1))) {
			codewordsInts[i / divisor] = codewordBytes[i + start];
		}
	}

	if (!ReedSolomonDecoder::Decode(GenericGF::MaxiCodeField64(), codewordsInts, ecCodewords / divisor))
		return false;

	// Copy back into array of bytes -- only need to worry about the bytes that were data
	// We don't care about errors in the error-correction codewords
	for (int i = 0; i < dataCodewords; i++) {
		if ((mode == ALL) || (i % 2 == (mode - 1))) {
			codewordBytes[i + start] = static_cast<uint8_t>(codewordsInts[i / divisor]);
		}
	}
	return true;
}

/**
* <p>MaxiCodes can encode text or structured information as bits in one of several modes,
* with multiple character sets in one code. This class decodes the bits back into text.</p>
*
* @author mike32767
* @author Manuel Kasten
*/
namespace DecodedBitStreamParser
{
	static const char SHI0 = 0x40;
	static const char SHI1 = 0x41;
	static const char SHI2 = 0x42;
	static const char SHI3 = 0x43;
	static const char SHI4 = 0x44;
	static const char TWSA = 0x45;		// two shift A
	static const char TRSA = 0x46;	// three shift A
	static const char LCHA = 0x47;		// latch A
	static const char LCHB = 0x48;		// latch B
	static const char LOCK = 0x49;
	static const char ECI  = 0x4A;
	static const char NS   = 0x4B;
	static const char PAD  = 0x4C;

	static const char FS = 0x1C;
	static const char GS = 0x1D;
	static const char RS = 0x1E;

	const static std::array<char, 0x40> CHARSETS[] = {
		{ // set 0
			'\n',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
			 'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  ECI,   FS,   GS,   RS,   NS,
			 ' ',  PAD,  '"',  '#',  '$',  '%',  '&', '\'',  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
			 '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':', SHI1, SHI2, SHI3, SHI4, LCHB,
		},
		{ // set 1
			 '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
			 'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  ECI,   FS,   GS,   RS,   NS,
			 '{',  PAD,  '}',  '~', 0x7F,  ';',  '<',  '=',  '>',  '?',  '[', '\\',  ']',  '^',  '_',  ' ',
			 ',',  '.',  '/',  ':',  '@',  '!',  '|',  PAD, TWSA, TRSA,  PAD, SHI0, SHI2, SHI3, SHI4, LCHA,
		},
		{ // set 2
			'\xC0', '\xC1', '\xC2', '\xC3', '\xC4', '\xC5', '\xC6', '\xC7', '\xC8', '\xC9', '\xCA', '\xCB', '\xCC', '\xCD', '\xCE', '\xCF',
			'\xD0', '\xD1', '\xD2', '\xD3', '\xD4', '\xD5', '\xD6', '\xD7', '\xD8', '\xD9', '\xDA',    ECI,     FS,     GS,     RS,     NS,		// Note that in original code in Java, NS is not there, which seems to be a bug
			'\xDB', '\xDC', '\xDD', '\xDE', '\xDF', '\xAA', '\xAC', '\xB1', '\xB2', '\xB3', '\xB5', '\xB9', '\xBA', '\xBC', '\xBD', '\xBE',
			'\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87', '\x88', '\x89',   LCHA, '\x20',   LOCK,   SHI3,   SHI4,   LCHB,
		},
		{ // set 3
			'\xE0', '\xE1', '\xE2', '\xE3', '\xE4', '\xE5', '\xE6', '\xE7', '\xE8', '\xE9', '\xEA', '\xEB', '\xEC', '\xED', '\xEE', '\xEF',
			'\xF0', '\xF1', '\xF2', '\xF3', '\xF4', '\xF5', '\xF6', '\xF7', '\xF8', '\xF9', '\xFA',    ECI,     FS,     GS,     RS,     NS,
			'\xFB', '\xFC', '\xFD', '\xFE', '\xFF', '\xA1', '\xA8', '\xAB', '\xAF', '\xB0', '\xB4', '\xB7', '\xB8', '\xBB', '\xBF', '\x8A',
			'\x8B', '\x8C', '\x8D', '\x8E', '\x8F', '\x90', '\x91', '\x92', '\x93', '\x94',   LCHA, '\x20',   SHI2,   LOCK,   SHI4,   LCHB,
		},
		{ // set 4
			'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0A', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F',
			'\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A',    ECI,    PAD,    PAD, '\x1B',     NS,
			    FS,     GS,     RS, '\x1F', '\x9F', '\xA0', '\xA2', '\xA3', '\xA4', '\xA5', '\xA6', '\xA7', '\xA9', '\xAD', '\xAE', '\xB6',
			'\x95', '\x96', '\x97', '\x98', '\x99', '\x9A', '\x9B', '\x9C', '\x9D', '\x9E',   LCHA, '\x20',   SHI2,   SHI3,   LOCK,   LCHB,
		},
		{ // set 5
			'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0A', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F',
			'\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F',
			'\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27', '\x28', '\x29', '\x2A', '\x2B', '\x2C', '\x2D', '\x2E', '\x2F',
			'\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', '\x38', '\x39', '\x3A', '\x3B', '\x3C', '\x3D', '\x3E', '\x3F',
		},
	};

	static int GetBit(int bit, const ByteArray& bytes)
	{
		bit--;
		return (bytes[bit / 6] & (1 << (5 - (bit % 6)))) == 0 ? 0 : 1;
	}

	static int GetInt(const ByteArray& bytes, const ByteArray& x)
	{
		int len = x.length();
		int val = 0;
		for (int i = 0; i < len; i++) {
			val += GetBit(x[i], bytes) << (len - i - 1);
		}
		return val;
	}

	static int GetPostCode2(const ByteArray& bytes)
	{
		return GetInt(bytes, { 33, 34, 35, 36, 25, 26, 27, 28, 29, 30, 19, 20, 21, 22, 23, 24, 13, 14, 15, 16, 17, 18, 7, 8, 9, 10, 11, 12, 1, 2 });
	}

	static int GetPostCode2Length(const ByteArray& bytes) {
		return GetInt(bytes, { 39, 40, 41, 42, 31, 32 });
	}
	
	static std::string GetPostCode3(const ByteArray& bytes)
	{
		return {
			CHARSETS[0].at(GetInt(bytes, { 39, 40, 41, 42, 31, 32 })),
			CHARSETS[0].at(GetInt(bytes, { 33, 34, 35, 36, 25, 26 })),
			CHARSETS[0].at(GetInt(bytes, { 27, 28, 29, 30, 19, 20 })),
			CHARSETS[0].at(GetInt(bytes, { 21, 22, 23, 24, 13, 14 })),
			CHARSETS[0].at(GetInt(bytes, { 15, 16, 17, 18,  7,  8 })),
			CHARSETS[0].at(GetInt(bytes, { 9,  10, 11, 12,  1,  2 })),
		};
	}


	static std::string ToString(int x, int width)
	{
		std::stringstream buf;
		buf << std::setw(width) << std::setfill('0') << x;
		return buf.str();
	}

	static int GetCountry(const ByteArray& bytes)
	{
		return GetInt(bytes, { 53, 54, 43, 44, 45, 46, 47, 48, 37, 38 });
	}

	static int GetServiceClass(const ByteArray& bytes)
	{
		return GetInt(bytes, { 55, 56, 57, 58, 59, 60, 49, 50, 51, 52 });
	}

	static std::string GetMessage(const ByteArray& bytes, int start, int len)
	{
		std::string sb;
		int shift = -1;
		int set = 0;
		int lastset = 0;
		for (int i = start; i < start + len; i++) {
			char c = CHARSETS[set].at(bytes[i]);
			switch (c) {
			case LCHA:
				set = 0;
				shift = -1;
				break;
			case LCHB:
				set = 1;
				shift = -1;
				break;
			case SHI0:
			case SHI1:
			case SHI2:
			case SHI3:
			case SHI4:
				lastset = set;
				set = c - SHI0;
				shift = 1;
				break;
			case TWSA:
				lastset = set;
				set = 0;
				shift = 2;
				break;
			case TRSA:
				lastset = set;
				set = 0;
				shift = 3;
				break;
			case NS:
				sb.append(ToString((bytes[i+1] << 24) + (bytes[i+2] << 18) + (bytes[i+3] << 12) + (bytes[i+4] << 6) + bytes[i+5], 9));
				i += 5;
				break;
			case LOCK:
				shift = -1;
				break;
			default:
				sb.push_back(c);
			}
			if (shift-- == 0) {
				set = lastset;
			}
		}
		while (sb.length() > 0 && sb.at(sb.length() - 1) == PAD) {
			sb.resize(sb.length() - 1);
		}
		return sb;
	}

	static DecoderResult Decode(ByteArray&& bytes, int mode)
	{
		std::string result;
		result.reserve(144);
		switch (mode) {
			case 2:
			case 3: {
				auto postcode = mode == 2 ? ToString(GetPostCode2(bytes), GetPostCode2Length(bytes)) : GetPostCode3(bytes);
				auto country = ToString(GetCountry(bytes), 3);
				auto service = ToString(GetServiceClass(bytes), 3);
				result.append(GetMessage(bytes, 10, 84));
				if (result.compare(0, 7, std::string("[)>") + RS + std::string("01") + GS) == 0) {
					result.insert(9, postcode + GS + country + GS + service + GS);
				}
				else {
					result.insert(0, postcode + GS + country + GS + service + GS);
				}
				break;
			}
			case 4:
				result.append(GetMessage(bytes, 1, 93));
				break;
			case 5:
				result.append(GetMessage(bytes, 1, 77));
				break;
		}
		return DecoderResult(std::move(bytes), TextDecoder::FromLatin1(result)).setEcLevel(std::to_wstring(mode)); // really???
	}



} // DecodedBitStreamParser

DecoderResult
Decoder::Decode(const BitMatrix& bits)
{
	ByteArray codewords = BitMatrixParser::ReadCodewords(bits);

	if (!CorrectErrors(codewords, 0, 10, 10, ALL)) {
		return DecodeStatus::ChecksumError;
	}
	int mode = codewords[0] & 0x0F;
	ByteArray datawords;
	switch (mode) {
		case 2:
		case 3:
		case 4:
			if (CorrectErrors(codewords, 20, 84, 40, EVEN) && CorrectErrors(codewords, 20, 84, 40, ODD)) {
				datawords.resize(94, 0);
			}
			else {
				return DecodeStatus::ChecksumError;
			}
			break;
		case 5:
			if (CorrectErrors(codewords, 20, 68, 56, EVEN) && CorrectErrors(codewords, 20, 68, 56, ODD)) {
				datawords.resize(78, 0);
			}
			else {
				return DecodeStatus::ChecksumError;
			}
			break;
		default:
			return DecodeStatus::FormatError;
	}

	std::copy_n(codewords.begin(), 10, datawords.begin());
	std::copy_n(codewords.begin() + 20, datawords.size() - 10, datawords.begin() + 10);

	return DecodedBitStreamParser::Decode(std::move(datawords), mode);
}

} // MaxiCode
} // ZXing
