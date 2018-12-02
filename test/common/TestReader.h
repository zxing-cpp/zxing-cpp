#pragma once
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

#include <string>
#include <map>
#include <memory>

namespace ZXing {

class DecodeHints;
class MultiFormatReader;
class BinaryBitmap;

namespace Test {

class ImageLoader;
    
class TestReader
{
public:
    struct ReadResult
    {
        std::string format, text;
        operator bool() const { return !format.empty(); }
    };

    TestReader(const std::shared_ptr<ImageLoader>& imgLoader, const DecodeHints& hints);
    ~TestReader();

    ReadResult read(const std::wstring& filename, int rotation = 0, bool isPure = false) const;

    static void clearCache();
    
private:
    std::shared_ptr<ImageLoader> _imageLoader;
    std::shared_ptr<MultiFormatReader> _reader;
    static std::map<std::wstring, std::shared_ptr<BinaryBitmap>> _cache;
};

}} // ZXing::Test
