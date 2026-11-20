// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"
#include "WXTestUtility.h"
#ifdef WXCODE_TEST_C_API
#include "ZXingC.h"
#endif

#include "gtest/gtest.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <numbers>
#include <string_view>
#include <vector>

using namespace ZXing;
using WXCode::Test::IMAGE_SIZE;

namespace {

class WXCodeReaderTest : public ::testing::Test
{
protected:
	const ByteArray payload{'A', 0, 0xf0, 0x55, 0, 0};
	std::vector<uint8_t> pixels = WXCode::Test::Render(WXCode::Test::Encode(payload));
	ImageView image{pixels.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::Lum};
	ReaderOptions options = ReaderOptions().formats(BarcodeFormat::WXCode).tryDownscale(false).tryInvert(false);

	void CheckBarcode(const Barcode& code)
	{
		EXPECT_TRUE(code.isValid());
		EXPECT_EQ(code.format(), BarcodeFormat::WXCode);
		EXPECT_EQ(code.bytes(), payload);
		EXPECT_TRUE(code.symbologyIdentifier().empty());
		EXPECT_EQ(code.symbol().data(), nullptr);
	}
};

} // namespace

TEST_F(WXCodeReaderTest, BinaryPayloadAndPosition)
{
	auto code = ReadBarcode(image, options);
	CheckBarcode(code);
	EXPECT_NEAR(code.position().topLeft().x, 17, 3);
	EXPECT_NEAR(code.position().topLeft().y, 17, 3);
	CheckBarcode(ReadBarcode(image));
	CheckBarcode(ReadBarcode(image, ReaderOptions(options).isPure(true)));
	CheckBarcode(ReadBarcode(image, ReaderOptions(options).tryDenoise(true)));
	CheckBarcode(ReadBarcode(image, ReaderOptions(options).tryDownscale(true)));
	EXPECT_TRUE(ReadBarcodes(image, ReaderOptions(options).formats(BarcodeFormat::QRCode)).empty());
}

TEST_F(WXCodeReaderTest, AllVersionsAndProfiles)
{
	for (unsigned version = 0; version < 4; ++version) {
		for (unsigned ec = 0; ec < 4; ++ec) {
			SCOPED_TRACE(::testing::Message() << "version=" << version << " ec=" << ec);
			auto raster = WXCode::Test::Render(WXCode::Test::Encode(payload, version, ec));
			CheckBarcode(ReadBarcode(ImageView(raster.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::Lum), options));
		}
	}
}

TEST_F(WXCodeReaderTest, RotatedViews)
{
	for (int angle : {90, 180, 270}) {
		SCOPED_TRACE(angle);
		CheckBarcode(ReadBarcode(image.rotated(angle), options));
	}
}

TEST_F(WXCodeReaderTest, RasterizedRotation)
{
	// Rotate pixels, rather than only changing ImageView's row/pixel strides.
	for (int degrees : {30, 120, 210, 300, -30}) {
		SCOPED_TRACE(degrees);
		double angle = degrees * std::numbers::pi / 180;
		double sine = std::sin(angle);
		double cosine = std::cos(angle);
		std::vector<uint8_t> rotated(IMAGE_SIZE * IMAGE_SIZE, 231);
		for (int y = 0; y < IMAGE_SIZE; ++y) {
			for (int x = 0; x < IMAGE_SIZE; ++x) {
				int sx = static_cast<int>(std::lround(cosine * (x - 152) + sine * (y - 152) + 152));
				int sy = static_cast<int>(std::lround(-sine * (x - 152) + cosine * (y - 152) + 152));
				if (sx >= 0 && sx < IMAGE_SIZE && sy >= 0 && sy < IMAGE_SIZE)
					rotated[y * IMAGE_SIZE + x] = pixels[sy * IMAGE_SIZE + sx];
			}
		}
		auto results = ReadBarcodes(ImageView(rotated.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::Lum), options);
		ASSERT_EQ(results.size(), 1U);
		CheckBarcode(results.front());
	}
}

TEST_F(WXCodeReaderTest, ImageLayouts)
{
	std::vector<uint8_t> padded(314 * IMAGE_SIZE, 123);
	std::vector<uint8_t> rgb(IMAGE_SIZE * IMAGE_SIZE * 3);
	for (int y = 0; y < IMAGE_SIZE; ++y)
		for (int x = 0; x < IMAGE_SIZE; ++x) {
			padded[y * 314 + x] = pixels[y * IMAGE_SIZE + x];
			for (int c = 0; c < 3; ++c)
				rgb[(y * IMAGE_SIZE + x) * 3 + c] = pixels[y * IMAGE_SIZE + x];
		}
	CheckBarcode(ReadBarcode(ImageView(padded.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::Lum, 314), options));
	CheckBarcode(ReadBarcode(ImageView(rgb.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::RGB), options));
	CheckBarcode(ReadBarcode(ImageView(rgb.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::RGB),
							 ReaderOptions(options).binarizer(Binarizer::FixedThreshold)));
	CheckBarcode(ReadBarcode(ImageView(rgb.data(), IMAGE_SIZE, IMAGE_SIZE, ImageFormat::Lum, IMAGE_SIZE * 3, 3), options));
}

TEST_F(WXCodeReaderTest, ImageLimit)
{
	std::vector<uint8_t> oversized(16385 * 3, 255);
	EXPECT_TRUE(ReadBarcodes(ImageView(oversized.data(), 16385, 3, ImageFormat::Lum), options).empty());
}

TEST_F(WXCodeReaderTest, LocalAverageUsesOtsuOnLowContrastImage)
{
	// Hybrid binarization discards this contrast; Otsu separates the two intensities.
	for (auto& pixel : pixels)
		pixel = pixel < 128 ? 100 : 101;
	CheckBarcode(ReadBarcode(image, options));
	CheckBarcode(ReadBarcode(image, ReaderOptions(options).binarizer(Binarizer::LocalAverage)));
}

TEST_F(WXCodeReaderTest, FixedThresholdOnWhiteCanvas)
{
	constexpr int width = 640;
	std::vector<uint8_t> canvas(width * width, 255);
	for (int y = 0; y < IMAGE_SIZE; ++y)
		for (int x = 0; x < IMAGE_SIZE; ++x)
			canvas[(y + 100) * width + x + 100] = pixels[y * IMAGE_SIZE + x] < 100 ? 100 : 145;
	ImageView canvasImage(canvas.data(), width, width, ImageFormat::Lum);
	auto fixed = ReaderOptions(options).binarizer(Binarizer::FixedThreshold).tryInvert(true);
	CheckBarcode(ReadBarcode(canvasImage, fixed));

	for (auto& pixel : canvas)
		pixel = 255 - pixel;
	auto inverted = ReadBarcode(canvasImage, fixed);
	CheckBarcode(inverted);
	EXPECT_TRUE(inverted.isInverted());
}

TEST_F(WXCodeReaderTest, Inversion)
{
	for (auto& pixel : pixels)
		pixel = 255 - pixel;
	EXPECT_TRUE(ReadBarcodes(image, options).empty());
	auto result = ReadBarcode(image, ReaderOptions(options).tryInvert(true));
	CheckBarcode(result);
	EXPECT_TRUE(result.isInverted());
}

TEST_F(WXCodeReaderTest, MultipleSymbolsAndBudget)
{
	std::vector<uint8_t> pair(640 * IMAGE_SIZE, 255);
	for (int y = 0; y < IMAGE_SIZE; ++y)
		for (int x = 0; x < IMAGE_SIZE; ++x)
			pair[y * 640 + x] = pair[y * 640 + 335 + x] = pixels[y * IMAGE_SIZE + x];
	ImageView pairImage(pair.data(), 640, IMAGE_SIZE, ImageFormat::Lum);
	auto results = ReadBarcodes(pairImage, ReaderOptions(options).tryDownscale(true));
	ASSERT_EQ(results.size(), 2U);
	for (const auto& result : results)
		CheckBarcode(result);
	EXPECT_EQ(ReadBarcodes(pairImage, ReaderOptions(options).maxNumberOfSymbols(1)).size(), 1U);
}

TEST_F(WXCodeReaderTest, PreservesCandidatesAtSamplingLimit)
{
	constexpr int tileSize = IMAGE_SIZE;
	constexpr int width = tileSize * 4;
	std::vector<uint8_t> sheet(width * width);
	for (int y = 0; y < width; ++y)
		for (int x = 0; x < width; x += tileSize)
			std::copy_n(pixels.begin() + (y % tileSize) * tileSize, tileSize, sheet.begin() + y * width + x);

	ImageView sheetImage(sheet.data(), width, width, ImageFormat::Lum);
	auto results = ReadBarcodes(sheetImage, options);
	ASSERT_EQ(results.size(), 16U);
	for (const auto& result : results)
		CheckBarcode(result);
	CheckBarcode(ReadBarcode(sheetImage, options));
}

#if ZXING_ENABLE_QRCODE
TEST_F(WXCodeReaderTest, MixedQRCode)
{
	// Standard 21-module QR fixture from the reference regression suite.
	static constexpr std::array<std::string_view, 21> QR = {
		"111111101100001111111", "100000100110101000001", "101110100101101011101", "101110101000101011101",
		"101110101011101011101", "100000101010101000001", "111111101010101111111", "000000001010000000000",
		"100010111000111111001", "000100001100111010000", "111001100100110011010", "001011000001110110011",
		"111001100011010011001", "000000001110100110100", "111111101111010010010", "100000100100010010000",
		"101110101011100101000", "101110100100011010111", "101110100100000011100", "100000100001010011000",
		"111111101001010110001",
	};
	constexpr int width = IMAGE_SIZE + (21 + 8) * 8;
	std::vector<uint8_t> mixed(width * IMAGE_SIZE, 255);
	for (int y = 0; y < IMAGE_SIZE; ++y)
		std::copy_n(pixels.begin() + y * IMAGE_SIZE, IMAGE_SIZE, mixed.begin() + y * width);
	for (int y = 0; y < 21; ++y)
		for (int x = 0; x < 21; ++x)
			if (QR[y][x] == '1')
				for (int yy = 0; yy < 8; ++yy)
					std::fill_n(mixed.begin() + ((y + 4) * 8 + yy) * width + IMAGE_SIZE + (x + 4) * 8, 8, 0);
	auto results = ReadBarcodes(ImageView(mixed.data(), width, IMAGE_SIZE, ImageFormat::Lum));
	ASSERT_EQ(results.size(), 2U);
	EXPECT_EQ(std::count_if(results.begin(), results.end(), [](const Barcode& b) { return b.format() == BarcodeFormat::WXCode; }),
			  1);
	EXPECT_EQ(std::count_if(results.begin(), results.end(), [](const Barcode& b) { return b.format() == BarcodeFormat::QRCode; }),
			  1);
}
#endif

#ifdef WXCODE_TEST_C_API
TEST_F(WXCodeReaderTest, CAPI)
{
	std::unique_ptr<ZXing_ReaderOptions, decltype(&ZXing_ReaderOptions_delete)> cOptions(ZXing_ReaderOptions_new(),
																						 ZXing_ReaderOptions_delete);
	ASSERT_NE(cOptions, nullptr);
	auto format = ZXing_BarcodeFormat_WXCode;
	ZXing_ReaderOptions_setFormats(cOptions.get(), &format, 1);
	std::unique_ptr<ZXing_ImageView, decltype(&ZXing_ImageView_delete)> cImage(
		ZXing_ImageView_new(pixels.data(), IMAGE_SIZE, IMAGE_SIZE, ZXing_ImageFormat_Lum, 0, 0), ZXing_ImageView_delete);
	ASSERT_NE(cImage, nullptr);
	std::unique_ptr<ZXing_Barcodes, decltype(&ZXing_Barcodes_delete)> results(ZXing_ReadBarcodes(cImage.get(), cOptions.get()),
																			  ZXing_Barcodes_delete);
	ASSERT_NE(results, nullptr);
	ASSERT_EQ(ZXing_Barcodes_size(results.get()), 1);
	const auto* code = ZXing_Barcodes_at(results.get(), 0);
	ASSERT_NE(code, nullptr);
	EXPECT_EQ(ZXing_Barcode_format(code), format);
	int length = 0;
	std::unique_ptr<uint8_t, decltype(&ZXing_free)> data(ZXing_Barcode_bytes(code, &length), ZXing_free);
	ASSERT_NE(data, nullptr);
	ASSERT_EQ(length, static_cast<int>(payload.size()));
	EXPECT_TRUE(std::equal(data.get(), data.get() + length, payload.begin()));
}
#endif
