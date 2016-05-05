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

#include "Binarizer.h"

namespace ZXing {

class LuminanceSource;

/**
* This Binarizer implementation uses the old ZXing global histogram approach. It is suitable
* for low-end mobile devices which don't have enough CPU or memory to use a local thresholding
* algorithm. However, because it picks a global black point, it cannot handle difficult shadows
* and gradients.
*
* Faster mobile devices and all desktop applications should probably use HybridBinarizer instead.
*
* @author dswitkin@google.com (Daniel Switkin)
* @author Sean Owen
*/
class GlobalHistogramBinarizer : public Binarizer
{
protected:
	std::shared_ptr<const LuminanceSource> _source;

public:
	GlobalHistogramBinarizer(const std::shared_ptr<const LuminanceSource>& source);

	virtual int width() const override;
	virtual int height() const override;
	virtual ErrorStatus getBlackRow(int y, BitArray& outArray) const override;
	virtual ErrorStatus getBlackMatrix(BitMatrix& outMatrix) const override;
	virtual bool canCrop() const override;
	virtual std::shared_ptr<Binarizer> cropped(int left, int top, int width, int height) const override;
	virtual bool canRotate() const override;
	virtual std::shared_ptr<Binarizer> rotatedCCW90() const override;
	virtual std::shared_ptr<Binarizer> rotatedCCW45() const override;

	virtual std::shared_ptr<Binarizer> createBinarizer(const std::shared_ptr<const LuminanceSource>& source) const;
};

} // ZXing
