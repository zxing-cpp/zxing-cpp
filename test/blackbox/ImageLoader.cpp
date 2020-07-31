/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggersauser.
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

#include "ImageLoader.h"
#include "BinaryBitmap.h"
#include "GenericLuminanceSource.h"

#include <memory>
#include <stdexcept>

#if 0
#include "GlobalHistogramBinarizer.h"
using Binarizer = ZXing::GlobalHistogramBinarizer;
#else
#include "HybridBinarizer.h"
using Binarizer = ZXing::HybridBinarizer;
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ZXing::Test {

std::map<fs::path, std::unique_ptr<BinaryBitmap>> ImageLoader::cache;

static std::shared_ptr<GenericLuminanceSource> readImage(const fs::path& imgPath)
{
	int width, height, colors;
	std::unique_ptr<stbi_uc, void (*)(void*)> buffer(stbi_load(imgPath.string().c_str(), &width, &height, &colors, 0),
													 stbi_image_free);
	if (buffer == nullptr) {
		throw std::runtime_error("Failed to read image");
	}
	switch (colors) {
	case 1: return std::make_shared<GenericLuminanceSource>(0, 0, width, height, buffer.get(), width, 1, 0, 0, 0, nullptr);
	case 2: return std::make_shared<GenericLuminanceSource>(0, 0, width, height, buffer.get(), width * colors, colors, 0, 0, 0, nullptr);
	case 3:
	case 4: return std::make_shared<GenericLuminanceSource>(0, 0, width, height, buffer.get(), width * colors, colors, 0, 1, 2, nullptr);
	}
	return {}; // silence warning
}

const BinaryBitmap& ImageLoader::load(const fs::path& imgPath)
{
	auto& binImg = cache[imgPath];
	if (binImg == nullptr)
		binImg = std::make_unique<Binarizer>(readImage(imgPath));
	return *binImg;
}

}
