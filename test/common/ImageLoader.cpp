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
	int width, height, channels;
	auto buffer = stbi_load(imgPath.string().c_str(), &width, &height, &channels, 3);
	if (buffer == nullptr) {
		throw std::runtime_error("Failed to read image");
	}
	auto lumSrc = std::make_shared<GenericLuminanceSource>(width, height, buffer, width * 3, 3, 0, 1, 2);
	stbi_image_free(buffer);
	return lumSrc;
}

const BinaryBitmap& ImageLoader::load(const fs::path& imgPath, bool isPure)
{
	auto& binImg = cache[imgPath];
	if (binImg == nullptr)
		binImg = std::make_unique<Binarizer>(readImage(imgPath), isPure);
	return *binImg;
}

}
