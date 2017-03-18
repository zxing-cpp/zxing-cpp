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

#include "CustomData.h"

#include <string>
#include <vector>

namespace ZXing {
namespace Pdf417 {

/**
* @author Guenther Grau
*/
class DecoderResultExtra : public CustomData
{
	int _segmentIndex;
	std::string _fileId;
	std::vector<int> _optionalData;
	bool _lastSegment;

public:

	int segmentIndex() const {
		return _segmentIndex;
	}

	void setSegmentIndex(int segmentIndex) {
		_segmentIndex = segmentIndex;
	}

	std::string fileId() const {
		return _fileId;
	}

	void setFileId(const std::string& fileId) {
		_fileId = fileId;
	}

	const std::vector<int>& getOptionalData() const {
		return _optionalData;
	}

	void setOptionalData(const std::vector<int>& optionalData) {
		_optionalData = optionalData;
	}

	bool isLastSegment() const {
		return _lastSegment;
	}

	void setLastSegment(bool lastSegment) {
		_lastSegment = lastSegment;
	}
};

} // Pdf417
} // ZXing
