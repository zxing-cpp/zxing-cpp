// SPDX-License-Identifier: Apache-2.0

#include "WXTestUtility.h"

#include "BitArray.h"
#include "wxcode/WXFormatInformation.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace ZXing::WXCode::Test {
namespace {

uint8_t GaloisMultiply(unsigned a, unsigned b)
{
	unsigned result = 0;
	while (b) {
		if (b & 1)
			result ^= a;
		a <<= 1;
		if (a & 256)
			a ^= 0x11d;
		b >>= 1;
	}
	return static_cast<uint8_t>(result);
}

// Independent polynomial encoder; no calls to the decoder's correction code.
ByteArray EncodeReedSolomon(ByteArray data, unsigned parity)
{
	ByteArray polynomial{1};
	uint8_t root = 1;
	for (unsigned i = 0; i < parity; ++i) {
		ByteArray next(static_cast<int>(polynomial.size() + 1));
		for (size_t j = 0; j < polynomial.size(); ++j) {
			next[j] ^= polynomial[j];
			next[j + 1] ^= GaloisMultiply(polynomial[j], root);
		}
		polynomial = std::move(next);
		root = GaloisMultiply(root, 2);
	}
	auto work = data;
	work.resize(data.size() + parity);
	for (size_t i = 0; i < data.size(); ++i) {
		auto coefficient = work[i];
		for (size_t j = 0; j < polynomial.size(); ++j)
			work[i + j] ^= GaloisMultiply(coefficient, polynomial[j]);
	}
	data.insert(data.end(), work.end() - parity, work.end());
	return data;
}

} // namespace

Grid EncodeData(const ByteArray& data, unsigned version, unsigned ec)
{
	const auto* profile = VersionEC(version, ec);
	if (!profile || data.size() != profile->blockCount * profile->dataCodewords)
		throw std::invalid_argument("Wrong WX test data size");

	std::vector<ByteArray> blocks;
	for (unsigned i = 0; i < profile->blockCount; ++i) {
		ByteArray block;
		block.assign(data.begin() + i * profile->dataCodewords, data.begin() + (i + 1) * profile->dataCodewords);
		blocks.push_back(EncodeReedSolomon(std::move(block), profile->ecCodewords));
	}
	BitArray stream;
	for (unsigned column = 0; column < profile->dataCodewords + profile->ecCodewords; ++column)
		for (const auto& block : blocks)
			stream.appendBits(block[column], 8);

	const auto word = FormatWord((version << 7) | ((ec ^ 1) << 5));
	BitArray format;
	format.appendBits(word, FORMAT_COPY_BITS);
	format.appendBits(word, FORMAT_COPY_BITS);
	const auto dataMask = VersionDataMask(version);
	const auto formatMask = FormatMask();
	Grid grid{};
	int dataCursor = 0;
	int formatCursor = 0;
	for (unsigned angle = 0; angle < ANGULAR_ROWS; ++angle) {
		for (unsigned radius = 0; radius < RADIAL_CELLS; ++radius) {
			if (dataMask[angle] & (1U << radius)) {
				const unsigned mask = 1 ^ ((angle + radius) & 1) ^ (angle >= 10 && angle <= 50);
				grid[angle][radius] = stream.get(dataCursor++) ^ mask;
			}
			if (formatMask[angle] & (1U << radius))
				grid[angle][radius] = format.get(formatCursor++);
		}
	}
	return grid;
}

Grid Encode(const ByteArray& payload, unsigned version, unsigned ec)
{
	const auto* profile = VersionEC(version, ec);
	if (!profile)
		throw std::invalid_argument("Invalid WX test profile");
	int capacity = static_cast<int>(profile->blockCount * profile->dataCodewords * 8);
	BitArray bits;
	bits.appendBits(4, 4);
	bits.appendBits(static_cast<int>(payload.size()), 8);
	for (auto byte : payload)
		bits.appendBits(byte, 8);
	if (bits.size() > capacity)
		throw std::invalid_argument("WX test payload exceeds capacity");
	bits.appendBits(0, std::min(4, capacity - bits.size()));
	while (bits.size() % 8)
		bits.appendBit(false);
	auto data = bits.toBytes();
	for (unsigned i = 0; data.size() < static_cast<size_t>(capacity / 8); ++i)
		data.push_back(i % 2 ? 0x11 : 0xec);
	return EncodeData(data, version, ec);
}

std::vector<uint8_t> Render(const Grid& grid)
{
	std::vector<uint8_t> image(IMAGE_SIZE * IMAGE_SIZE, 231);
	constexpr double offset = 15.909903;
	for (int y = 0; y < IMAGE_SIZE; ++y) {
		for (int x = 0; x < IMAGE_SIZE; ++x) {
			double dx = (x - 152) / 5.0;
			double dy = (y - 152) / 5.0;
			double distance = std::min({std::hypot(dx + offset, dy + offset), std::hypot(dx - offset, dy + offset),
										std::hypot(dx + offset, dy - offset)});
			bool ink;
			if (distance < 3.4) {
				ink = distance < .5 || (distance > 1.7 && distance < 2.5);
			} else {
				int radius = static_cast<int>(std::floor(std::hypot(dx, dy))) - 10;
				int angle = (static_cast<int>(std::floor(std::atan2(-dy, -dx) * 36 / std::numbers::pi + .5)) + 72) % 72;
				ink = radius >= 0 && radius < RADIAL_CELLS && grid[angle][radius];
			}
			image[y * IMAGE_SIZE + x] = ink ? 35 : 231;
		}
	}
	return image;
}

} // namespace ZXing::WXCode::Test
