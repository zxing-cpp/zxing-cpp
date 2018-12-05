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
#include <vector>
#include <memory>
#include "TestReader.h"

namespace ZXing {

namespace Test {

class ImageLoader;

class Pdf417MultipleCodeReader
{
public:
    struct ReadResult : public TestReader::ReadResult
	{
        std::vector<std::string> fileIds;
	};

	Pdf417MultipleCodeReader(const std::shared_ptr<ImageLoader>& imgLoader);

	ReadResult readMultiple(const std::vector<std::wstring>& filenames, int rotation = 0) const;

private:
	std::shared_ptr<ImageLoader> _imageLoader;
};


}} // ZXing::Test
