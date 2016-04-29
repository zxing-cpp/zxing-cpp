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
#include "DetectorResult.h"

namespace ZXing {
namespace Aztec {

class DetectorResult : public ZXing::DetectorResult
{
	bool _compact;
	int _nbDatablocks;
	int _nbLayers;

public:
	DetectorResult() : _compact(false), _nbDatablocks(0), _nbLayers(0) {}
	/*DetectorResult(const BitMatrix& bits, const std::vector<ResultPoint>& points, bool compact, int nbDatablocks, int nbLayers) :
		ZXing::DetectorResult(bits, points),
		_compact(compact),
		_nbDatablocks(nbDatablocks),
		_nbLayers(nbLayers)
	{
	}*/

	bool isCompact() const { return _compact; }
	void setCompact(bool compact) { _compact = compact; }

	int nbDatablocks() const { return _nbDatablocks; }
	void setNbDatablocks(int nb) { _nbDatablocks = nb; }

	int nbLayers() const { return _nbLayers; }
	void setNbLayers(int nb) { _nbLayers = nb; }
};

} // Aztec
} // ZXing
