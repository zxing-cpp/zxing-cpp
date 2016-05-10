#pragma once
/*
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
#include <memory>

namespace ZXing {

class StringCodecs;

namespace Pdf417 {

/**
* This implementation can detect and decode PDF417 codes in an image.
*
* @author Guenther Grau
*/
class Reader : public ZXing::Reader
{
public:
	explicit Reader(const std::shared_ptr<const StringCodecs>& codec);

	virtual Result decode(const BinaryBitmap& image) const override;

public:
	std::shared_ptr<const StringCodecs> _codec;
};

} // Pdf417
} // ZXing
