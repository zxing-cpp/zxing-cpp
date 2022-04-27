#ifndef __DETECTOR_H__
#define __DETECTOR_H__

/*
 * Copyright 2017 KURZ Digital Solutions GmbH & Co. KG
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

#include "BitMatrix.h"
#include "DetectorResult.h"
#include "FinderPatternInfo.h"
#include "PerspectiveTransform.h"

namespace ZXing {

class DecodeHints;

namespace MicroQRCode {

class Detector
{
public:
	Detector(const BitMatrix& image);
	DetectorResult detect(DecodeHints const& hints) const;

private: // methods
	DetectorResult detectPure() const;
	DetectorResult processCodeEnclosingRect(const std::vector<ResultPoint>& codeEnclosingRect, int dimension) const;
	DetectorResult processFinderPatternInfo(const FinderPatternInfo& patternInfo, int dimension) const;
	PerspectiveTransform createTransform(const std::vector<ResultPoint>& rect, int dimension) const;
	PerspectiveTransform createTransform(const ResultPoint& topLeft, const ResultPoint& topRight,
										 const ResultPoint& bottomLeft, int dimension) const;
	DetectorResult sampleGrid(const BitMatrix& image, const PerspectiveTransform& transform, int dimension) const;
	int computeDimension(const ResultPoint& topLeft, const ResultPoint& topRight, const ResultPoint& bottomLeft,
						 float moduleSize) const;
	int computeDimension(const std::vector<ResultPoint>& codeEnclosingRect, float moduleSize) const;

private:
	BitMatrix image_;
};

} // namespace MicroQRCode

} // namespace ZXing

#endif // __DETECTOR_H__
