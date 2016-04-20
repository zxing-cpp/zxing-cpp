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

#include "Reader.h"

namespace ZXing {

/**
* MultiFormatReader is a convenience class and the main entry point into the library for most uses.
* By default it attempts to decode all barcode formats that the library supports. Optionally, you
* can provide a hints object to request different behavior, for example only decoding QR codes.
*
* @author Sean Owen
* @author dswitkin@google.com (Daniel Switkin)
*/
class MultiFormatReader : public Reader
{
public:
	/**
	* Decode an image using the hints provided. Does not honor existing state.
	*
	* @param image The pixel data to decode
	* @param hints The hints to use, clearing the previous state.
	* @return The contents of the image
	* @throws NotFoundException Any errors which occurred
	*/
	virtual Result decode(const BinaryBitmap& image, const DecodeHints* hints = nullptr) const override;

};

} // ZXing
