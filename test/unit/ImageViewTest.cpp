// SPDX-License-Identifier: Apache-2.0

#include "ImageView.h"

#include "gtest/gtest.h"

using namespace ZXing;

TEST(ImageViewTest, BoundsChecks)
{
	uint8_t data = 0;

	EXPECT_THROW(ImageView(&data, -1, 1, 1, ImageFormat::Lum), std::invalid_argument);
	EXPECT_THROW(ImageView(&data, 2, 2, 1, ImageFormat::Lum, 1), std::invalid_argument);
	EXPECT_THROW(ImageView(&data, 3, 1, 1, ImageFormat::RGB, 3, 2), std::invalid_argument);
	EXPECT_THROW(ImageView(&data, 1, 1, 2, ImageFormat::Lum), std::invalid_argument);

	// overflow checks
	EXPECT_THROW(ImageView(&data, 0, 32768, 32768, ImageFormat::Lum, 0, 131072), std::invalid_argument);
	EXPECT_THROW(ImageView(&data, 0, 1, 32768, ImageFormat::Lum, 131072), std::invalid_argument);
}
