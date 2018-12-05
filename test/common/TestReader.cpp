/*
* Copyright 2016 Nu-book Inc.
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

#include "TestReader.h"
#include "HybridBinarizer.h"
#include "MultiFormatReader.h"
#include "DecodeHints.h"
#include "Result.h"
#include "ImageLoader.h"

#if 0
using Binarizer = ZXing::GlobalHistogramBinarizer;
#else
using Binarizer = ZXing::HybridBinarizer;
#endif

namespace ZXing { namespace Test {

std::map<std::wstring, std::shared_ptr<BinaryBitmap>> TestReader::_cache;

void
TestReader::clearCache()
{
    _cache.clear();
}

TestReader::TestReader(const std::shared_ptr<ImageLoader>& imgLoader, const DecodeHints& hints)
: _imageLoader(imgLoader), _reader(std::make_shared<MultiFormatReader>(hints))
{
}

TestReader::ReadResult
TestReader::read(const std::wstring& filename, int rotation, bool isPure) const
{
    auto& binImg = _cache[filename];
    if (binImg == nullptr)
        binImg = std::make_shared<Binarizer>(_imageLoader->load(filename), isPure);
    auto result = _reader->read(*binImg->rotated(rotation));
    if (result.isValid()) {
        return { ToString(result.format()), result.text() };
    }
    return {};
}

}} // ZXing::Test
