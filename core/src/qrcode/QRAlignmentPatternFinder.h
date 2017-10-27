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

namespace ZXing {

class BitMatrix;

namespace QRCode {

class AlignmentPattern;

/**
* <p>This class attempts to find alignment patterns in a QR Code. Alignment patterns look like finder
* patterns but are smaller and appear at regular intervals throughout the image.</p>
*
* <p>At the moment this only looks for the bottom-right alignment pattern.</p>
*
* <p>This is mostly a simplified copy of {@link FinderPatternFinder}. It is copied,
* pasted and stripped down here for maximum performance but does unfortunately duplicate
* some code.</p>
*
* <p>This class is thread-safe but not reentrant. Each thread must allocate its own object.</p>
*
* @author Sean Owen
*/
class AlignmentPatternFinder
{
public:
	/**
	* <p>This method attempts to find the bottom-right alignment pattern in the image. It is a bit messy since
	* it's pretty performance-critical and so is written to be fast foremost.</p>
	* @param image image to search
	* @param startX left column from which to start searching
	* @param startY top row from which to start searching
	* @param width width of region to search
	* @param height height of region to search
	* @param moduleSize estimated module size so far
	*
	* @return {@link AlignmentPattern} if found
	* @throws NotFoundException if not found
	*/
	static AlignmentPattern Find(const BitMatrix& image, int startX, int startY, int width, int height, float moduleSize);
};

} // QRCode
} // ZXing
