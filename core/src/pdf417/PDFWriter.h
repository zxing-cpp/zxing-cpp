#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include <string>
#include <memory>

namespace ZXing {

class BitMatrix;
enum class CharacterSet;

namespace Pdf417 {

enum class Compaction;
class Encoder;

/**
* @author Jacob Haynes
* @author qwandor@google.com (Andrew Walbran)
*/
class Writer
{
public:
	Writer();
	Writer(Writer &&);
	~Writer();

	Writer& setMargin(int margin) { _margin = margin; return *this; }

	Writer& setErrorCorrectionLevel(int ecLevel) { _ecLevel = ecLevel; return *this; }

	/**
	* Sets max/min row/col values
	*
	* @param maxCols maximum allowed columns
	* @param minCols minimum allowed columns
	* @param maxRows maximum allowed rows
	* @param minRows minimum allowed rows
	*/
	Writer& setDimensions(int minCols, int maxCols, int minRows, int maxRows);

	/**
	* @param compaction compaction mode to use
	*/
	Writer& setCompaction(Compaction compaction);

	/**
	* @param compact if true, enables compaction
	*/
	Writer& setCompact(bool compact);

	/**
	* @param encoding sets character encoding to use
	*/
	Writer& setEncoding(CharacterSet encoding);


	BitMatrix encode(const std::wstring& contents, int width, int height) const;

private:
	int _margin = -1;
	int _ecLevel = -1;
	std::unique_ptr<Encoder> _encoder;
};

} // Pdf417
} // ZXing
