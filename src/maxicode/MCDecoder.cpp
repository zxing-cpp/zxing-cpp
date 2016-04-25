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

#include "maxicode/MCDecoder.h"
#include "maxicode/MCBitMatrixParser.h"
#include "ByteArray.h"
#include "DecoderResult.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"

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
			codewordsInts[i / divisor] = codewordBytes[i + start] & 0xFF;
		}
	}

	if (StatusIsOK(ReedSolomonDecoder(GenericGF::MaxiCodeField64()).decode(codewordsInts, ecCodewords / divisor))) {
		// Copy back into array of bytes -- only need to worry about the bytes that were data
		// We don't care about errors in the error-correction codewords
		for (int i = 0; i < dataCodewords; i++) {
			if ((mode == ALL) || (i % 2 == (mode - 1))) {
				codewordBytes[i + start] = static_cast<uint8_t>(codewordsInts[i / divisor]);
			}
		}
		return true;
	}
	return false;
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
	static const uint16_t SHIFTA = '\uFFF0';
	static const uint16_t SHIFTB = '\uFFF1';
	static const uint16_t SHIFTC = '\uFFF2';
	static const uint16_t SHIFTD = '\uFFF3';
	static const uint16_t SHIFTE = '\uFFF4';
	static const uint16_t TWOSHIFTA = '\uFFF5';
	static const uint16_t THREESHIFTA = '\uFFF6';
	static const uint16_t LATCHA = '\uFFF7';
	static const uint16_t LATCHB = '\uFFF8';
	static const uint16_t LOCK = '\uFFF9';
	static const uint16_t ECI = '\uFFFA';
	static const uint16_t NS = '\uFFFB';
	static const uint16_t PAD = '\uFFFC';
	static const uint16_t FS = '\u001C';
	static const uint16_t GS = '\u001D';
	static const uint16_t RS = '\u001E';

	const static char CHARSET_0[] = {
		'\n','A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ECI,  FS,  GS,  RS, NS ,
		' ', PAD, '"', '#', '$', '%', '&','\'', '(', ')', '*', '+', ',', '-', '.', '/',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', SHIFTB, SHIFTC, SHIFTD, SHIFTE, LATCHB,
	};

	const static char CHARSET_2[64] = {
		0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
		0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,  ECI,   FS,   GS,   RS, 0xDB,
		0xDC, 0xDD, 0xDE, 0xDF, 0xAA, 0xAC, 0xB1, 0xB2, 0xB3, 0xB5, 0xB9, 0xBA, 0xBC, 0xBD, 0xBE, 0x80,
		0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,LATCHA,0x20, LOCK,SHIFTD,SHIFTE,LATCHB,
	};

	const static char CHARSET_3[64] = {
		0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
		0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,  ECI,   FS,   GS,   RS,   NS,
		0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0xA1, 0xA8, 0xAB, 0xAF, 0xB0, 0xB4, 0xB7, 0xB8, 0xBB, 0xBF, 0x8A,
		0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,LATCHA,0x20,SHIFTC,LOCK,SHIFTE,LATCHB,
	};

	const static char CHARSET_4[64] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,  ECI,  PAD,  PAD, 0x1B,   NS,
		  FS,   GS,   RS, 0x1F, 0x9F, 0xA0, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA9, 0xAD, 0xAE, 0xB6,
		0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, LATCHA,0x20,SHIFTC,SHIFTD,LOCK,LATCHB,
	};

	const static char CHARSET_5[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	};

	private static final String[] SETS = {
		"\nABCDEFGHIJKLMNOPQRSTUVWXYZ" + ECI + FS + GS + RS + NS + ' ' + PAD + "\"#$%&'()*+,-./0123456789:" + SHIFTB + SHIFTC + SHIFTD + SHIFTE + LATCHB,
		"`abcdefghijklmnopqrstuvwxyz" + ECI + FS + GS + RS + NS + '{' + PAD + "}~\u007F;<=>?[\\]^_ ,./:@!|" + PAD + TWOSHIFTA + THREESHIFTA + PAD + SHIFTA + SHIFTC + SHIFTD + SHIFTE + LATCHA,
		"\u00C0\u00C1\u00C2\u00C3\u00C4\u00C5\u00C6\u00C7\u00C8\u00C9\u00CA\u00CB\u00CC\u00CD\u00CE\u00CF\u00D0\u00D1\u00D2\u00D3\u00D4\u00D5\u00D6\u00D7\u00D8\u00D9\u00DA" + ECI + FS + GS + RS + "\u00DB\u00DC\u00DD\u00DE\u00DF\u00AA\u00AC\u00B1\u00B2\u00B3\u00B5\u00B9\u00BA\u00BC\u00BD\u00BE\u0080\u0081\u0082\u0083\u0084\u0085\u0086\u0087\u0088\u0089" + LATCHA + ' ' + LOCK + SHIFTD + SHIFTE + LATCHB,
		"\u00E0\u00E1\u00E2\u00E3\u00E4\u00E5\u00E6\u00E7\u00E8\u00E9\u00EA\u00EB\u00EC\u00ED\u00EE\u00EF\u00F0\u00F1\u00F2\u00F3\u00F4\u00F5\u00F6\u00F7\u00F8\u00F9\u00FA" + ECI + FS + GS + RS + NS + "\u00FB\u00FC\u00FD\u00FE\u00FF\u00A1\u00A8\u00AB\u00AF\u00B0\u00B4\u00B7\u00B8\u00BB\u00BF\u008A\u008B\u008C\u008D\u008E\u008F\u0090\u0091\u0092\u0093\u0094" + LATCHA + ' ' + SHIFTC + LOCK + SHIFTE + LATCHB,
		"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u0008\u0009\n\u000B\u000C\r\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A" + ECI + PAD + PAD + '\u001B' + NS + FS + GS + RS + "\u001F\u009F\u00A0\u00A2\u00A3\u00A4\u00A5\u00A6\u00A7\u00A9\u00AD\u00AE\u00B6\u0095\u0096\u0097\u0098\u0099\u009A\u009B\u009C\u009D\u009E" + LATCHA + ' ' + SHIFTC + SHIFTD + LOCK + LATCHB,
		"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u0008\u0009\n\u000B\u000C\r\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F\u0020\u0021\"\u0023\u0024\u0025\u0026\u0027\u0028\u0029\u002A\u002B\u002C\u002D\u002E\u002F\u0030\u0031\u0032\u0033\u0034\u0035\u0036\u0037\u0038\u0039\u003A\u003B\u003C\u003D\u003E\u003F"
	};



	private DecodedBitStreamParser() {
	}

	static DecoderResult decode(byte[] bytes, int mode) {
		StringBuilder result = new StringBuilder(144);
		switch (mode) {
		case 2:
		case 3:
			String postcode;
			if (mode == 2) {
				int pc = getPostCode2(bytes);
				NumberFormat df = new DecimalFormat("0000000000".substring(0, getPostCode2Length(bytes)));
				postcode = df.format(pc);
			}
			else {
				postcode = getPostCode3(bytes);
			}
			NumberFormat threeDigits = new DecimalFormat("000");
			String country = threeDigits.format(getCountry(bytes));
			String service = threeDigits.format(getServiceClass(bytes));
			result.append(getMessage(bytes, 10, 84));
			if (result.toString().startsWith("[)>" + RS + "01" + GS)) {
				result.insert(9, postcode + GS + country + GS + service + GS);
			}
			else {
				result.insert(0, postcode + GS + country + GS + service + GS);
			}
			break;
		case 4:
			result.append(getMessage(bytes, 1, 93));
			break;
		case 5:
			result.append(getMessage(bytes, 1, 77));
			break;
		}
		return new DecoderResult(bytes, result.toString(), null, String.valueOf(mode));
	}

	private static int getBit(int bit, byte[] bytes) {
		bit--;
		return (bytes[bit / 6] & (1 << (5 - (bit % 6)))) == 0 ? 0 : 1;
	}

	private static int getInt(byte[] bytes, byte[] x) {
		if (x.length == 0) {
			throw new IllegalArgumentException();
		}
		int val = 0;
		for (int i = 0; i < x.length; i++) {
			val += getBit(x[i], bytes) << (x.length - i - 1);
		}
		return val;
	}

	private static int getCountry(byte[] bytes) {
		return getInt(bytes, new byte[]{ 53, 54, 43, 44, 45, 46, 47, 48, 37, 38 });
	}

	private static int getServiceClass(byte[] bytes) {
		return getInt(bytes, new byte[]{ 55, 56, 57, 58, 59, 60, 49, 50, 51, 52 });
	}

	private static int getPostCode2Length(byte[] bytes) {
		return getInt(bytes, new byte[]{ 39, 40, 41, 42, 31, 32 });
	}

	private static int getPostCode2(byte[] bytes) {
		return getInt(bytes, new byte[]{ 33, 34, 35, 36, 25, 26, 27, 28, 29, 30, 19,
			20, 21, 22, 23, 24, 13, 14, 15, 16, 17, 18, 7, 8, 9, 10, 11, 12, 1, 2 });
	}

	private static String getPostCode3(byte[] bytes) {
		return String.valueOf(
			new char[] {
			SETS[0].charAt(getInt(bytes, new byte[]{ 39, 40, 41, 42, 31, 32 })),
				SETS[0].charAt(getInt(bytes, new byte[]{ 33, 34, 35, 36, 25, 26 })),
				SETS[0].charAt(getInt(bytes, new byte[]{ 27, 28, 29, 30, 19, 20 })),
				SETS[0].charAt(getInt(bytes, new byte[]{ 21, 22, 23, 24, 13, 14 })),
				SETS[0].charAt(getInt(bytes, new byte[]{ 15, 16, 17, 18,  7,  8 })),
				SETS[0].charAt(getInt(bytes, new byte[]{ 9, 10, 11, 12,  1,  2 })),
		}
		);
	}

	private static String getMessage(byte[] bytes, int start, int len) {
		StringBuilder sb = new StringBuilder();
		int shift = -1;
		int set = 0;
		int lastset = 0;
		for (int i = start; i < start + len; i++) {
			char c = SETS[set].charAt(bytes[i]);
			switch (c) {
			case LATCHA:
				set = 0;
				shift = -1;
				break;
			case LATCHB:
				set = 1;
				shift = -1;
				break;
			case SHIFTA:
			case SHIFTB:
			case SHIFTC:
			case SHIFTD:
			case SHIFTE:
				lastset = set;
				set = c - SHIFTA;
				shift = 1;
				break;
			case TWOSHIFTA:
				lastset = set;
				set = 0;
				shift = 2;
				break;
			case THREESHIFTA:
				lastset = set;
				set = 0;
				shift = 3;
				break;
			case NS:
				int nsval = (bytes[++i] << 24) + (bytes[++i] << 18) + (bytes[++i] << 12) + (bytes[++i] << 6) + bytes[++i];
				sb.append(new DecimalFormat("000000000").format(nsval));
				break;
			case LOCK:
				shift = -1;
				break;
			default:
				sb.append(c);
			}
			if (shift-- == 0) {
				set = lastset;
			}
		}
		while (sb.length() > 0 && sb.charAt(sb.length() - 1) == PAD) {
			sb.setLength(sb.length() - 1);
		}
		return sb.toString();
	}

} // DecodedBitStreamParser

DecoderResult
Decoder::Decode(const BitMatrix& bits, const DecodeHints* hints)
{
	ByteArray codewords;
	BitMatrixParser::ReadCodewords(bits, codewords);

	if (!CorrectErrors(codewords, 0, 10, 10, ALL)) {
		return DecoderResult(ErrorStatus::ChecksumError);
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
				return DecoderResult(ErrorStatus::ChecksumError);
			}
			break;
		case 5:
			if (CorrectErrors(codewords, 20, 68, 56, EVEN) && CorrectErrors(codewords, 20, 68, 56, ODD)) {
				datawords.resize(78, 0);
			}
			else {
				return DecoderResult(ErrorStatus::ChecksumError);
			}
			break;
		default:
			return DecoderResult(ErrorStatus::FormatError);
	}

	std::copy_n(codewords.begin(), 10, datawords.begin());
	std::copy_n(codewords.begin() + 20, datawords.size() - 10, datawords.begin() + 10);

	return DecodedBitStreamParser.decode(datawords, mode);
}

} // MaxiCode
} // ZXing
