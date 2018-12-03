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
	int _segmentIndex = 0;
	std::string _fileId;
	std::vector<int> _optionalData;
	bool _lastSegment = false;
	int _segmentCount = -1;
	std::string _sender;
	std::string _addressee;
	std::string _fileName;
	int64_t _fileSize = -1;
	int64_t _timestamp = -1;
	int _checksum = -1;

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

	const std::vector<int>& optionalData() const {
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

	int segmentCount() const {
		return _segmentCount;
	}

	void setSegmentCount(int segmentCount) {
		_segmentCount = segmentCount;
	}

	std::string sender() const {
		return _sender;
	}

	void setSender(const std::string& sender) {
		_sender = sender;
	}

	std::string addressee() const {
		return _addressee;
	}

	void setAddressee(const std::string& addressee) {
		_addressee = addressee;
	}

	std::string fileName() const {
		return _fileName;
	}

	void setFileName(const std::string& fileName) {
		_fileName = fileName;
	}

	// -1 if not set
	int64_t fileSize() const {
		return _fileSize;
	}

	void setFileSize(int64_t fileSize) {
		_fileSize = fileSize;
	}

	 // 16-bit CRC checksum using CCITT-16, -1 if not set
	int checksum() const {
		return _checksum;
	}

	void setChecksum(int checksum) {
		_checksum = checksum;
	}

	// Unix epock timestamp, elapsed seconds since 1970-01-01, -1 if not set
	int64_t timestamp() const {
		return _timestamp;
	}

	void setTimestamp(int64_t timestamp) {
		_timestamp = timestamp;
	}
};

} // Pdf417
} // ZXing
