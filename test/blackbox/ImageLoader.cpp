/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggersauser.
*/
// SPDX-License-Identifier: Apache-2.0

#include "ImageLoader.h"

#include "BinaryBitmap.h"
#include "ImageView.h"

#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <webp/decode.h>

namespace ZXing::Test {

class STBImage : public ImageView
{
	std::unique_ptr<uint8_t[], void (*)(void*)> _memory = {nullptr, nullptr};

public:
	void load(const fs::path& imgPath)
	{
		int width = 0, height = 0, channels = 0;
		int rowStride = 0;

		if (imgPath.extension() == ".webp") {
			std::ifstream file(imgPath, std::ios::binary);
			if (!file)
				throw std::runtime_error("Failed to read image: " + imgPath.string() + " (failed to open file)");

			std::vector<uint8_t> compressedData(std::istreambuf_iterator<char>(file), {});
			if (compressedData.empty())
				throw std::runtime_error("Failed to read image: " + imgPath.string() + " (empty file)");

#if 0
			// directly decoding to YUV is faster and makes more sense but a couple of tests fail with this, so we decode to RGB and convert to Lum instead
			channels = 1;
			uint8_t *u = nullptr, *v = nullptr;
			int uvStride = 0;
			_memory = std::unique_ptr<uint8_t[], void (*)(void*)>(
				WebPDecodeYUV(compressedData.data(), compressedData.size(), &width, &height, &u, &v, &rowStride, &uvStride), WebPFree);
#else
			channels = 3;
			_memory = std::unique_ptr<uint8_t[], void (*)(void*)>(
				WebPDecodeRGB(compressedData.data(), compressedData.size(), &width, &height), WebPFree);
#endif
			if (_memory == nullptr)
				throw std::runtime_error("Failed to read image: " + imgPath.string() + " (WebP decode failed)");

		} else {
			_memory = std::unique_ptr<uint8_t[], void (*)(void*)>(stbi_load(imgPath.string().c_str(), &width, &height, &channels, 0), stbi_image_free);
			if (_memory == nullptr)
				throw std::runtime_error("Failed to read image: " + imgPath.string() + " (" + stbi_failure_reason() + ")");
			rowStride = width * channels;
		}

		auto ImageFormatFromChannels = std::array{ImageFormat::None, ImageFormat::Lum, ImageFormat::LumA, ImageFormat::RGB, ImageFormat::RGBA};
		ImageView::operator=({_memory.get(), width, height, ImageFormatFromChannels.at(channels), rowStride});

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
std::mutex cacheMutex;

void ImageLoader::clearCache()
{
	std::scoped_lock lock(cacheMutex);
	cache.clear();
}

const ImageView& ImageLoader::load(const fs::path& imgPath)
{
	std::scoped_lock lock(cacheMutex);
	auto& binImg = cache[imgPath];
	if (!binImg)
		binImg.load(imgPath);

	return binImg;
}

} // namespace ZXing::Test
