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
/*
 * As we only have one FinderPattern in a Micro QR Code we will make up
 * the to others. All calculations are made with the information of the one
 * real FinderPattern. If this is wrong the calculated centers will be wrong as well.
 * It is also highly vulnerable for perspective transformed Micro QR Codes.
 *
 * @author Christian Braun
 * @author Ralph Corby
 */

#include "FakeCenterCalculator.h"

#include "Dimension.h"

using ZXing::ResultPoint;
using ZXing::MicroQRCode::FakeCenterCalculator;
using ZXing::MicroQRCode::FinderPattern;

/**
 *
 * @param actualCenter The center which was found
 * @param rect can only work with a correctly rotated Micro QR Code
 */
FakeCenterCalculator::FakeCenterCalculator(const FinderPattern& actualCenter, const std::vector<ResultPoint>& rect)
	: actualCenter_(actualCenter), moduleSize_(actualCenter.getEstimatedModuleSize()), rect_(rect), dimension_(0)
{
}

FinderPattern FakeCenterCalculator::getTopRightCenter()
{
	ResultPoint deltas = calculateNormalizedDeltas(rect_[0], rect_[2]);
	ResultPoint center = calculateCenter(deltas);

	return FinderPattern{center.x(), center.y(), moduleSize_};
}

FinderPattern FakeCenterCalculator::getBottomLeftCenter()
{
	ResultPoint deltas = calculateNormalizedDeltas(rect_[0], rect_[1]);
	ResultPoint center = calculateCenter(deltas);

	return FinderPattern{center.x(), center.y(), moduleSize_};
}

ResultPoint FakeCenterCalculator::calculateCenter(const ResultPoint& deltas) const
{
	int modulesBetweenCenters = dimension_ - 7;

	ResultPoint rightCenter{actualCenter_.x() + modulesBetweenCenters * moduleSize_ * deltas.x(),
							actualCenter_.y() + modulesBetweenCenters * moduleSize_ * deltas.y()};

	return rightCenter;
}

ResultPoint FakeCenterCalculator::calculateNormalizedDeltas(const ResultPoint& source, const ResultPoint& destination)
{
	float distance = ResultPoint::Distance(std::lroundf(source.x()), std::lroundf(source.y()),
										   std::lroundf(destination.x()), std::lroundf(destination.y()));
	float estimatedDimension = distance / moduleSize_;

	dimension_ = ZXing::MicroQRCode::Dimension::computeRoundUp((int)std::round(estimatedDimension));

	float deltaX = destination.x() - source.x();
	float deltaY = destination.y() - source.y();

	float normalizedDeltaX = deltaX / distance;
	float normalizedDeltaY = deltaY / distance;

	return ResultPoint{normalizedDeltaX, normalizedDeltaY};
}
