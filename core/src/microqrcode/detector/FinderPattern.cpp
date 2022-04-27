/*
 * Copyright 2007 ZXing authors All rights reserved.
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

#include "FinderPattern.h"

using ZXing::MicroQRCode::FinderPattern;

FinderPattern::FinderPattern(float posX, float posY, float estimatedModuleSize)
	: ResultPoint(posX, posY), estimatedModuleSize_(estimatedModuleSize), count_(1)
{
}

FinderPattern::FinderPattern(float posX, float posY, float estimatedModuleSize, int count)
	: ResultPoint(posX, posY), estimatedModuleSize_(estimatedModuleSize), count_(count)
{
}

/**
 * Get the number of patterns contributing to this estimate.
 *
 * @return the number of patterns that contribute to this estimate of the pattern position
 * and module size
 */
int FinderPattern::getCount() const
{
	return count_;
}

float FinderPattern::getEstimatedModuleSize() const
{
	return estimatedModuleSize_;
}

/**
 * Checks to see if a pattern postion and module size matches an existing one.
 *
 * @param moduleSize size of the module derived from the found pattern
 * @param i vertical position
 * @param j horizontal position
 * @return true if the pattern position and module size is similar to the existing position/size.
 */
bool FinderPattern::aboutEquals(float moduleSize, float i, float j) const
{
	if (std::abs(i - y()) <= moduleSize && abs(j - x()) <= moduleSize) {
		float moduleSizeDiff = abs(moduleSize - estimatedModuleSize_);
		return moduleSizeDiff <= 1.0f || moduleSizeDiff <= estimatedModuleSize_;
	}
	return false;
}

/**
 * Combine the estimate of a pattern postion and module size with an existing one.
 *
 * @param i vertical position
 * @param j horizontal position
 * @param newModuleSize size of the module derived from the found pattern
 * @return the combined estimate of the center of the pattern
 */
FinderPattern FinderPattern::combineEstimate(float i, float j, float newModuleSize) const
{
	int combinedCount = count_ + 1;
	float combinedX = (count_ * x() + j) / combinedCount;
	float combinedY = (count_ * y() + i) / combinedCount;
	float combinedModuleSize = (count_ * getEstimatedModuleSize() + newModuleSize) / combinedCount;
	return FinderPattern{combinedX, combinedY, combinedModuleSize, combinedCount};
}
