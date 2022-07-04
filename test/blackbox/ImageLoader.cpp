/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggersauser.
*/
// SPDX-License-Identifier: Apache-2.0

#include "ImageLoader.h"

#include "BinaryBitmap.h"
#include "ImageView.h"

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
	STBImage() : ImageView(nullptr, 0, 0, ImageFormat::None), _memory(nullptr, stbi_image_free) {}

	void load(const fs::path& imgPath)
	{
		int width, height, colors;
		_memory.reset(stbi_load(imgPath.string().c_str(), &width, &height, &colors, 3));
		if (_memory == nullptr)
			throw std::runtime_error("Failed to read image");
#if 1
		auto* img = _memory.get();
		for (int i = 0; i < width * height; ++i )
			img[i] = RGBToLum(img[3 * i + 0], img[3 * i + 1], img[3 * i + 2]);
		ImageView::operator=({_memory.get(), width, height, ImageFormat::Lum});
#else
		ImageView::operator=({_memory.get(), width, height, ImageFormat::RGB});
#endif
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
