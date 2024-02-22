/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggersauser.
*/
// SPDX-License-Identifier: Apache-2.0

#include "ImageLoader.h"

#include "BinaryBitmap.h"
#include "ImageView.h"

#include <array>
#include <map>
#include <memory>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ZXing::Test {

class STBImage : public ImageView
{
	std::unique_ptr<stbi_uc[], void (*)(void*)> _memory;

public:
	STBImage() : ImageView(), _memory(nullptr, stbi_image_free) {}

	void load(const fs::path& imgPath)
	{
		int width, height, channels;
		_memory.reset(stbi_load(imgPath.string().c_str(), &width, &height, &channels, 0));
		if (_memory == nullptr)
			throw std::runtime_error("Failed to read image: " + imgPath.string() + " (" + stbi_failure_reason() + ")");

		auto ImageFormatFromChannels = std::array{ImageFormat::None, ImageFormat::Lum, ImageFormat::LumA, ImageFormat::RGB, ImageFormat::RGBA};
		ImageView::operator=({_memory.get(), width, height, ImageFormatFromChannels.at(channels)});

		// preconvert from RGB -> Lum to do this only once instead of for each rotation
		if (_format == ImageFormat::RGB) {
			auto* img = _memory.get();
			for (int i = 0; i < width * height; ++i)
				img[i] = RGBToLum(img[3 * i + 0], img[3 * i + 1], img[3 * i + 2]);
			ImageView::operator=({_memory.get(), width, height, ImageFormat::Lum});
		}
	}

	operator bool() const { return _data; }
};

std::map<fs::path, STBImage> cache;

void ImageLoader::clearCache()
{
	cache.clear();
}

const ImageView& ImageLoader::load(const fs::path& imgPath)
{
	thread_local std::unique_ptr<BinaryBitmap> localAverage, threshold;

	auto& binImg = cache[imgPath];
	if (!binImg)
		binImg.load(imgPath);

	return binImg;
}

} // namespace ZXing::Test
