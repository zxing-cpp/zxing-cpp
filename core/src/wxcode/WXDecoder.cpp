// SPDX-License-Identifier: Apache-2.0

#include "WXDecoder.h"

#include "BitSource.h"
#include "WXConstants.h"
#include "WXFormatInformation.h"
#include "WXPayload.h"

#include <algorithm>
#include <array>
#include <span>

namespace ZXing::WXCode {

// Maximum codewords and parity bytes across all four version profiles.
static constexpr size_t MAX_WORDS = 66;
static constexpr size_t MAX_PARITY = 24;

static uint8_t GaloisMultiply(uint8_t a, uint8_t b)
{
	unsigned product = 0;
	unsigned factor = a;
	while (b) {
		if (b & 1)
			product ^= factor;
		factor <<= 1;
		if (factor & 256)
			factor ^= 0x11d;
		b >>= 1;
	}
	return static_cast<uint8_t>(product);
}

static uint8_t GaloisPower(uint8_t base, unsigned exponent)
{
	uint8_t result = 1;
	while (exponent) {
		if (exponent & 1)
			result = GaloisMultiply(result, base);
		base = GaloisMultiply(base, base);
		exponent >>= 1;
	}
	return result;
}

static bool HasNonzeroSyndromes(std::span<const uint8_t> block, std::span<uint8_t> syndrome)
{
	uint8_t any = 0;
	uint8_t root = 1;
	for (auto& syndromeValue : syndrome) {
		uint8_t value = 0;
		for (auto codeword : block)
			value = GaloisMultiply(value, root) ^ codeword;
		syndromeValue = value;
		any |= value;
		root = GaloisMultiply(root, 2);
	}
	return any;
}

static bool CorrectBlock(std::span<uint8_t> block, unsigned parity)
{
	std::array<uint8_t, MAX_PARITY> syndromeStorage{};
	auto syndrome = std::span(syndromeStorage).first(parity);
	if (!HasNonzeroSyndromes(block, syndrome))
		return true;

	/* Berlekamp-Massey, with the locator stored in ascending powers. */
	std::array<uint8_t, MAX_PARITY + 1> locator{1};
	std::array<uint8_t, MAX_PARITY + 1> previous{1};
	uint8_t priorDelta = 1;
	unsigned degree = 0;
	unsigned shift = 1;
	for (unsigned i = 0; i < parity; i++) {
		uint8_t delta = syndrome[i];
		for (unsigned j = 1; j <= degree; j++)
			delta ^= GaloisMultiply(locator[j], syndrome[i - j]);
		if (!delta) {
			shift++;
			continue;
		}
		auto saved = locator;
		uint8_t scale = GaloisMultiply(delta, GaloisPower(priorDelta, 254));
		for (unsigned j = shift; j <= parity; j++)
			locator[j] ^= GaloisMultiply(scale, previous[j - shift]);
		if (2 * degree <= i) {
			degree = i + 1 - degree;
			previous = saved;
			priorDelta = delta;
			shift = 1;
		} else {
			shift++;
		}
	}
	if (!degree || 2 * degree > parity)
		return false;

	/* Locate damaged symbols, then solve S_i = sum(error_j * X_j^i). */
	std::array<unsigned, MAX_PARITY / 2> positions{};
	unsigned found = 0;
	std::array<std::array<uint8_t, MAX_PARITY / 2 + 1>, MAX_PARITY / 2> matrix{};
	for (unsigned j = 0; j < block.size(); j++) {
		uint8_t x = GaloisPower(2, static_cast<unsigned>(block.size()) - 1 - j);
		uint8_t inverse = GaloisPower(x, 254);
		uint8_t value = locator[degree];
		for (unsigned k = degree; k; k--)
			value = GaloisMultiply(value, inverse) ^ locator[k - 1];
		if (value)
			continue;
		if (found == degree)
			return false;
		positions[found] = j;
		value = 1;
		for (unsigned k = 0; k < degree; k++) {
			matrix[k][found] = value;
			value = GaloisMultiply(value, x);
		}
		found++;
	}
	if (found != degree)
		return false;
	for (unsigned i = 0; i < degree; i++)
		matrix[i][degree] = syndrome[i];
	/* All leading minors are nonzero for distinct Vandermonde columns. */
	for (unsigned i = 0; i < degree; i++) {
		if (!matrix[i][i])
			return false;
		uint8_t inverse = GaloisPower(matrix[i][i], 254);
		for (unsigned k = i; k <= degree; k++)
			matrix[i][k] = GaloisMultiply(matrix[i][k], inverse);
		for (unsigned j = 0; j < degree; j++) {
			if (j == i)
				continue;
			uint8_t factor = matrix[j][i];
			for (unsigned k = i; k <= degree; k++)
				matrix[j][k] ^= GaloisMultiply(factor, matrix[i][k]);
		}
	}
	for (unsigned i = 0; i < degree; i++)
		block[positions[i]] ^= matrix[i][degree];
	return !HasNonzeroSyndromes(block, syndrome);
}

static int ReadBits(BitSource& reader, unsigned count)
{
	if (count > static_cast<unsigned>(reader.available()))
		return -1;
	return count ? reader.readBits(static_cast<int>(count)) : 0;
}

enum class Mode
{
	Terminator = 0,
	Numeric = 1,
	Alphanumeric = 2,
	StructuredAppend = 3,
	Byte = 4,
	FNC1FirstPosition = 5,
	ECI = 7,
	Kanji = 8,
	FNC1SecondPosition = 9,
	Hanzi = 13,
};

static bool ReadECI(BitSource& reader)
{
	int first = ReadBits(reader, 8);
	if (first < 0 || first >= 224)
		return false;
	auto assignment = static_cast<unsigned>(first);
	if (first >= 128) {
		unsigned tailBits = first < 192 ? 8 : 16;
		int tail = ReadBits(reader, tailBits);
		if (tail < 0)
			return false;
		assignment = ((assignment & (tailBits == 8 ? 63u : 31u)) << tailBits) | static_cast<unsigned>(tail);
	}
	// Validate the engine's ECI registry without changing the payload encoding.
	return assignment == 170 || (assignment <= 30 && assignment != 14 && assignment != 19);
}

static bool DecodeNumericSegment(BitSource& reader, int count, ByteArray& payload)
{
	while (count > 0) {
		int groupSize = std::min(count, 3);
		int value = ReadBits(reader, groupSize == 3 ? 10 : groupSize == 2 ? 7 : 4);
		if (value < 0 || value >= (groupSize == 3 ? 1000 : groupSize == 2 ? 100 : 10))
			return false;
		auto previousSize = payload.size();
		payload.resize(previousSize + groupSize);
		for (int i = groupSize; i; --i) {
			payload[previousSize + i - 1] = static_cast<uint8_t>('0' + value % 10);
			value /= 10;
		}
		count -= groupSize;
	}
	return true;
}

static bool DecodeAlphanumericSegment(BitSource& reader, int count, ByteArray& payload)
{
	while (count > 0) {
		bool pair = count >= 2;
		int value = ReadBits(reader, pair ? 11 : 6);
		constexpr int alphabetSize = ALPHANUMERIC_ALPHABET.size();
		if (value < 0 || value >= (pair ? alphabetSize * alphabetSize : alphabetSize))
			return false;
		if (pair)
			payload.push_back(static_cast<uint8_t>(ALPHANUMERIC_ALPHABET[value / alphabetSize]));
		payload.push_back(static_cast<uint8_t>(ALPHANUMERIC_ALPHABET[value % alphabetSize]));
		count -= pair ? 2 : 1;
	}
	return true;
}

static bool DecodeByteSegment(BitSource& reader, int count, unsigned byteWidth, ByteArray& payload)
{
	while (count-- > 0) {
		int value = ReadBits(reader, byteWidth);
		if (value < 0)
			return false;
		payload.push_back(static_cast<uint8_t>(value));
	}
	return true;
}

static bool DecodeDoubleByteSegment(BitSource& reader, int count, Mode mode, ByteArray& payload)
{
	while (count-- > 0) {
		int value = ReadBits(reader, 13);
		if (value < 0)
			return false;
		int divisor = mode == Mode::Kanji ? 192 : 96;
		value = (value / divisor) * 256 + value % divisor;
		if (mode == Mode::Kanji)
			value += value < 0x1f00 ? 0x8140 : 0xc140;
		else
			value += value < 0x03bf ? 0xa1a1 : 0xa6a1;
		payload.push_back(static_cast<uint8_t>(value >> 8));
		payload.push_back(static_cast<uint8_t>(value));
	}
	return true;
}

static void ApplyFNC1(ByteArray& payload, size_t segmentStart)
{
	size_t destination = segmentStart;
	for (size_t i = segmentStart; i < payload.size(); ++i) {
		uint8_t value = payload[i];
		if (value == '%') {
			if (i + 1 < payload.size() && payload[i + 1] == '%')
				++i;
			else
				value = 29;
		}
		payload[destination++] = value;
	}
	payload.resize(destination);
}

static ByteArray DecodeSegments(const ByteArray& data)
{
	BitSource reader(data);
	ByteArray payload;
	bool fnc1 = false;
	while (reader.available() >= 4) {
		auto mode = static_cast<Mode>(ReadBits(reader, 4));
		switch (mode) {
		case Mode::Terminator: return payload;
		case Mode::FNC1FirstPosition:
		case Mode::FNC1SecondPosition: fnc1 = true; continue;
		case Mode::StructuredAppend:
			if (ReadBits(reader, 16) < 0)
				return {};
			continue;
		case Mode::ECI:
			if (!ReadECI(reader))
				return {};
			continue;
		case Mode::Numeric:
		case Mode::Alphanumeric:
		case Mode::Byte:
		case Mode::Kanji:
		case Mode::Hanzi: break;
		default: return {};
		}

		int subset = mode == Mode::Hanzi ? ReadBits(reader, 4) : 1;
		int count = ReadBits(reader, mode == Mode::Numeric ? 10 : mode == Mode::Alphanumeric ? 9 : 8);
		if (count < 0 || subset < 0)
			return {};
		// Compatibility: unsupported Hanzi subsets consume only their header.
		if (subset != 1)
			continue;
		unsigned byteWidth = 8;
		if (mode == Mode::Byte) {
			unsigned available = reader.available();
			count = std::min(count, static_cast<int>((available + 7) / 8));
			// Only an initially short byte is accepted; later truncation still fails.
			if (available < 8)
				byteWidth = available;
		}
		size_t requiredBytes = static_cast<size_t>(count) * (mode == Mode::Kanji || mode == Mode::Hanzi ? 2 : 1);
		if (requiredBytes > MAX_PAYLOAD_BYTES - payload.size())
			return {};

		size_t segmentStart = payload.size();
		bool valid = false;
		switch (mode) {
		case Mode::Numeric: valid = DecodeNumericSegment(reader, count, payload); break;
		case Mode::Alphanumeric: valid = DecodeAlphanumericSegment(reader, count, payload); break;
		case Mode::Byte: valid = DecodeByteSegment(reader, count, byteWidth, payload); break;
		case Mode::Kanji:
		case Mode::Hanzi: valid = DecodeDoubleByteSegment(reader, count, mode, payload); break;
		default: break;
		}
		if (!valid)
			return {};
		if (mode == Mode::Alphanumeric) {
			if (fnc1)
				ApplyFNC1(payload, segmentStart);
			// Conversion deliberately includes output from all preceding segments.
			if (!TransformPayload(payload))
				return {};
		}
	}
	return payload;
}

ByteArray Decode(const Grid& grid)
{
	auto formatMask = FormatMask();
	unsigned position = 0;
	std::array<unsigned, 2> copies{};
	for (unsigned angle = 0; angle < ANGULAR_ROWS; angle++) {
		for (unsigned radius = 0; radius < RADIAL_CELLS; radius++) {
			if (grid[angle][radius] > 1)
				return {};
			if ((formatMask[angle] >> radius) & 1) {
				auto& copy = copies[position++ / FORMAT_COPY_BITS];
				copy = (copy << 1) | (grid[angle][radius] & 1);
			}
		}
	}
	FormatInformation format{};
	if (!DecodeFormat(copies[0], copies[1], format))
		return {};
	const auto* profile = VersionEC(format.version, format.ecBits ^ 1);
	auto mask = VersionDataMask(format.version);
	unsigned total = VersionTotalCodewords(format.version);
	std::array<uint8_t, MAX_WORDS> words{};
	ByteArray data(static_cast<int>(profile->blockCount * profile->dataCodewords));
	position = 0;
	for (unsigned angle = 0; angle < ANGULAR_ROWS; angle++) {
		for (unsigned radius = 0; radius < RADIAL_CELLS && position < total * 8; radius++) {
			if ((mask[angle] >> radius) & 1) {
				unsigned bit = grid[angle][radius] ^ MaskBit(format.mask, angle, radius);
				words[position / 8] |= static_cast<uint8_t>((bit & 1) << (7 - position % 8));
				position++;
			}
		}
	}
	if (position != total * 8)
		return {};
	unsigned blockSize = profile->dataCodewords + profile->ecCodewords;
	for (unsigned i = 0; i < profile->blockCount; i++) {
		std::array<uint8_t, MAX_WORDS> block{};
		for (unsigned j = 0; j < blockSize; j++)
			block[j] = words[j * profile->blockCount + i];
		if (!CorrectBlock(std::span(block).first(blockSize), profile->ecCodewords))
			return {};
		std::copy_n(block.begin(), profile->dataCodewords, data.begin() + i * profile->dataCodewords);
	}
	return DecodeSegments(data);
}

} // namespace ZXing::WXCode
