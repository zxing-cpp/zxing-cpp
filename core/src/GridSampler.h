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

#include <memory>

namespace ZXing {

class BitMatrix;
class PerspectiveTransform;

/**
* Implementations of this class can, given locations of finder patterns for a QR code in an
* image, sample the right points in the image to reconstruct the QR code, accounting for
* perspective distortion. It is abstracted since it is relatively expensive and should be allowed
* to take advantage of platform-specific optimized implementations, like Sun's Java Advanced
* Imaging library, but which may not be available in other environments such as J2ME, and vice
* versa.
*
* The implementation used can be controlled by calling {@link #setGridSampler(GridSampler)}
* with an instance of a class which implements this interface.
*
* @author Sean Owen
*/
class GridSampler
{
public:
	virtual ~GridSampler() {}

	/**
	* Samples an image for a rectangular matrix of bits of the given dimension. The sampling
	* transformation is determined by the coordinates of 4 points, in the original and transformed
	* image space.
	*
	* @param image image to sample
	* @param dimensionX width of {@link BitMatrix} to sample from image
	* @param dimensionY height of {@link BitMatrix} to sample from image
	* @param p1ToX point 1 preimage X
	* @param p1ToY point 1 preimage Y
	* @param p2ToX point 2 preimage X
	* @param p2ToY point 2 preimage Y
	* @param p3ToX point 3 preimage X
	* @param p3ToY point 3 preimage Y
	* @param p4ToX point 4 preimage X
	* @param p4ToY point 4 preimage Y
	* @param p1FromX point 1 image X
	* @param p1FromY point 1 image Y
	* @param p2FromX point 2 image X
	* @param p2FromY point 2 image Y
	* @param p3FromX point 3 image X
	* @param p3FromY point 3 image Y
	* @param p4FromX point 4 image X
	* @param p4FromY point 4 image Y
	* @return {@link BitMatrix} representing a grid of points sampled from the image within a region
	*   defined by the "from" parameters
	* @throws NotFoundException if image can't be sampled, for example, if the transformation defined
	*   by the given points is invalid or results in sampling outside the image boundaries
	*/
	virtual BitMatrix sampleGrid(const BitMatrix& image, int dimensionX, int dimensionY,
		float p1ToX, float p1ToY, float p2ToX, float p2ToY, float p3ToX, float p3ToY, float p4ToX, float p4ToY,
		float p1FromX, float p1FromY, float p2FromX, float p2FromY, float p3FromX, float p3FromY, float p4FromX, float p4FromY) const = 0;

	virtual BitMatrix sampleGrid(const BitMatrix& image, int dimensionX, int dimensionY, const PerspectiveTransform& transform) const = 0;

	static std::shared_ptr<GridSampler> Instance();
	static void SetInstance(const std::shared_ptr<GridSampler>& inst);
};

} // ZXing
