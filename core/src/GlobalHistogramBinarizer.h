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

#include "BinaryBitmap.h"

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
class GlobalHistogramBinarizer : public BinaryBitmap
{
protected:
	std::shared_ptr<const LuminanceSource> _source;
	bool _pureBarcode;

public:
	explicit GlobalHistogramBinarizer(std::shared_ptr<const LuminanceSource> source, bool pureBarcode = false);
	~GlobalHistogramBinarizer() override;

	bool isPureBarcode() const override;
	int width() const override;
	int height() const override;
	bool getBlackRow(int y, BitArray& row) const override;
	std::shared_ptr<const BitMatrix> getBlackMatrix() const override;
	bool canCrop() const override;
	std::shared_ptr<BinaryBitmap> cropped(int left, int top, int width, int height) const override;
	bool canRotate() const override;
	std::shared_ptr<BinaryBitmap> rotated(int degreeCW) const override;

	virtual std::shared_ptr<BinaryBitmap> newInstance(const std::shared_ptr<const LuminanceSource>& source) const;

private:
	struct DataCache;
	std::unique_ptr<DataCache> _cache;
};

} // ZXing
