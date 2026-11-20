// SPDX-License-Identifier: Apache-2.0

#include "wxcode/WXDetector.h"

#include "ImageView.h"
#include "WXTestUtility.h"

#include "gtest/gtest.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <vector>

using namespace ZXing::WXCode;

static constexpr int WIDTH = ZXing::WXCode::Test::IMAGE_SIZE;
static constexpr int STRIDE = 314;
static constexpr double pi = std::numbers::pi;
static constexpr double unit = 5.0, center = 152.0;
static constexpr double offset = 15.909903;

static int ReferenceBit(int angle, int radius)
{
	/* Deliberately non-periodic across quadrants and radii. */
	return ((angle * 19 + radius * 7 + (angle ^ radius) * 3) % 29) < 12;
}

static double FinderDistance(double x, double y, int numFinders)
{
	const std::array distances = {hypot(x + offset, y + offset), hypot(x - offset, y + offset), hypot(x + offset, y - offset)};
	double nearest = distances[0];
	for (int i = 1; i < numFinders; ++i)
		nearest = fmin(nearest, distances[i]);
	return nearest;
}

static void Draw(uint8_t* out, int numFinders)
{
	for (int y = 0; y < WIDTH; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			double dx = (x - center) / unit, dy = (y - center) / unit;
			double distance = FinderDistance(dx, dy, numFinders);
			int ink = 0;
			if (distance < 3.4) {
				ink = distance < .5 || (distance > 1.7 && distance < 2.5);
			} else {
				int radius = static_cast<int>(floor(hypot(dx, dy))) - 10;
				int angle = (static_cast<int>(lround(atan2(-dy, -dx) * 36 / pi)) + 72) % 72;
				if (radius >= 0 && radius < 16)
					ink = ReferenceBit(angle, radius);
			}
			out[y * STRIDE + x] = ink ? 35 : 231;
		}
	}
}

static void CheckDetection(const Detection& detection)
{
	int checked = 0;
	for (int a = 0; a < 72; ++a) {
		for (int r = 0; r < 16; ++r) {
			double theta = a * pi / 36, radius = 10.5 + r;
			if (FinderDistance(-cos(theta) * radius, -sin(theta) * radius, 3) < 5)
				continue;
			++checked;
			EXPECT_EQ(detection.bits[a][r], ReferenceBit(a, r)) << "angle " << a << " radius " << r;
		}
	}
	EXPECT_GE(checked, 1000);
	for (int i = 0; i < 4; ++i) {
		double x = center + (i == 1 || i == 2 ? 27 : -27) * unit;
		double y = center + (i >= 2 ? 27 : -27) * unit;
		// The axes in this fixture are canonical before rotating its view.
		EXPECT_LE(hypot(x - detection.position[i].x, y - detection.position[i].y), unit);
	}
}

TEST(WXCodeDetectorTest, PolarSamplingAndPaddedRows)
{
	std::vector<uint8_t> image((WIDTH - 1) * STRIDE + WIDTH);
	Draw(image.data(), 3);
	ZXing::ImageView view(image.data(), WIDTH, WIDTH, ZXing::ImageFormat::Lum, STRIDE);
	auto detections = Detect(view);
	ASSERT_EQ(detections.size(), 1U);
	CheckDetection(detections.front());
}

TEST(WXCodeDetectorTest, MissingFinder)
{
	std::vector<uint8_t> image((WIDTH - 1) * STRIDE + WIDTH);
	Draw(image.data(), 2);
	EXPECT_TRUE(Detect(ZXing::ImageView(image.data(), WIDTH, WIDTH, ZXing::ImageFormat::Lum, STRIDE)).empty());
}

TEST(WXCodeDetectorTest, RotatedViews)
{
	std::vector<uint8_t> image((WIDTH - 1) * STRIDE + WIDTH);
	Draw(image.data(), 3);
	ZXing::ImageView view(image.data(), WIDTH, WIDTH, ZXing::ImageFormat::Lum, STRIDE);
	auto expected = Detect(view);
	ASSERT_EQ(expected.size(), 1U);
	for (int angle : {90, 180, 270}) {
		auto detections = Detect(view.rotated(angle));
		// Detection returns candidates; the decoder rejects alternative axes.
		EXPECT_TRUE(std::any_of(detections.begin(), detections.end(), [&](const Detection& detection) {
			return detection.bits == expected.front().bits;
		})) << angle;
	}
}

TEST(WXCodeDetectorTest, ConsolidatesAdjacentScanlines)
{
	std::vector<uint8_t> source((WIDTH - 1) * STRIDE + WIDTH);
	Draw(source.data(), 3);
	// Larger central dots are encountered on several adjacent scanlines.
	for (int scale : {1, 2, 4}) {
		SCOPED_TRACE(scale);
		int size = WIDTH * scale;
		std::vector<uint8_t> image(size * size);
		for (int y = 0; y < size; ++y)
			for (int x = 0; x < size; ++x)
				image[y * size + x] = source[(y / scale) * STRIDE + x / scale];
		auto detections = Detect(ZXing::ImageView(image.data(), size, size, ZXing::ImageFormat::Lum));
		EXPECT_EQ(detections.size(), 1U);
	}
}

TEST(WXCodeDetectorTest, RejectsEmptyAndOversizedImagesBeforeReadingPixels)
{
	EXPECT_TRUE(Detect(ZXing::ImageView{}).empty());
	// Only a sentinel is needed: unsupported dimensions must be rejected before sampling.
	const uint8_t sentinel = 255;
	EXPECT_TRUE(Detect(ZXing::ImageView(&sentinel, 16385, 1, ZXing::ImageFormat::Lum)).empty());
	EXPECT_TRUE(Detect(ZXing::ImageView(&sentinel, 1, 16385, ZXing::ImageFormat::Lum)).empty());
	EXPECT_TRUE(Detect(ZXing::ImageView(&sentinel, 8192, 4097, ZXing::ImageFormat::Lum)).empty());
}
