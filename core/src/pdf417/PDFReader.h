#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "Reader.h"
#include <list>

namespace ZXing {
namespace Pdf417 {

/**
* This implementation can detect and decode PDF417 codes in an image.
*
* @author Guenther Grau
*/
class Reader : public ZXing::Reader
{
public:
	Result decode(const BinaryBitmap& image) const override;
	std::list<Result> decodeMultiple(const BinaryBitmap& image) const;
};

} // Pdf417
} // ZXing
