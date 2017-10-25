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
#include "DetectorResult.h"

namespace ZXing {
namespace Aztec {

class DetectorResult : public ZXing::DetectorResult
{
	bool _compact = false;
	int _nbDatablocks = 0;
	int _nbLayers = 0;

	DetectorResult(const DetectorResult&) = delete;
	DetectorResult& operator=(const DetectorResult&) = delete;

public:
	DetectorResult() = default;
	DetectorResult(DetectorResult&&) = default;
	DetectorResult& operator=(DetectorResult&&) = default;

	DetectorResult(BitMatrix&& bits, std::vector<ResultPoint>&& points, bool isCompact, int nbDatablocks, int nbLayers)
		: ZXing::DetectorResult{std::move(bits), std::move(points)}, _compact(isCompact), _nbDatablocks(nbDatablocks),
		  _nbLayers(nbLayers)
	{}

	bool isCompact() const { return _compact; }
	int nbDatablocks() const { return _nbDatablocks; }
	int nbLayers() const { return _nbLayers; }
};

} // Aztec
} // ZXing
