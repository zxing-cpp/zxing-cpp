/*
* Copyright 2016 Nu-book Inc.
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

#include "BarcodeReader.h"
#include "TextUtfEncoding.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "DecodeHints.h"
#include "ImageReader.h"

namespace ZXing {

BarcodeReader::BarcodeReader(bool tryHarder, bool tryRotate, const std::string& format)
{
	DecodeHints hints;
	hints.setTryHarder(tryHarder);
	hints.setTryRotate(tryRotate);
	hints.setFormats(BarcodeFormatsFromString(format));
	_reader = std::make_shared<MultiFormatReader>(hints);
}

BarcodeReader::ScanResult
BarcodeReader::scan(Gdiplus::Bitmap& bitmap, int rotations)
{
	Result result(DecodeStatus::NotFound);
	auto binImg = std::make_shared<HybridBinarizer>(ImageReader::Read(bitmap));
	
	if ((rotations & Rotation0) != 0) {
		result = _reader->read(*binImg);
	}
	if (!result.isValid() && (rotations & Rotation180) != 0) {
		result = _reader->read(*binImg->rotated(180));
	}
	if (!result.isValid() && (rotations & RotationCW90) != 0) {
		result = _reader->read(*binImg->rotated(90));
	}
	if (!result.isValid() && (rotations & RotationCCW90) != 0) {
		result = _reader->read(*binImg->rotated(270));
	}
	if (result.isValid()) {
		return{ ToString(result.format()), TextUtfEncoding::ToUtf8(result.text()) };
	}
	return ScanResult();
}

} // ZXing
