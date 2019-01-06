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

#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "DecodeHints.h"
#include "BarcodeFormat.h"
#include "lodepng.h"

#include <string>
#include <memory>
#include <emscripten/bind.h>

#if 0
using Binarizer = ZXing::GlobalHistogramBinarizer;
#else
using Binarizer = ZXing::HybridBinarizer;
#endif

struct ReadResult
{
    std::string format;
    std::wstring text;
    std::string error;
};

ReadResult readBarcodeFromPng(int bufferPtr, int bufferLength, bool tryHarder, std::string format)
{
    using namespace ZXing;
	try {
       	DecodeHints hints;
        hints.setShouldTryHarder(tryHarder);
        hints.setShouldTryRotate(tryHarder);
        auto fixedFormat = BarcodeFormatFromString(format);
        if (fixedFormat != BarcodeFormat::FORMAT_COUNT)
            hints.setPossibleFormats({ fixedFormat });
        MultiFormatReader reader(hints);

        std::vector<unsigned char> bitmap;
        unsigned width, height;
        unsigned error = lodepng::decode(bitmap, width, height, reinterpret_cast<const unsigned char*>(bufferPtr), size_t(bufferLength));
        if (error) {
            return { "", L"", lodepng_error_text(error) };
        }

        GenericLuminanceSource source((int)width, (int)height, bitmap.data(), width * 4, 4, 0, 1, 2);
        Binarizer binImage(std::shared_ptr<LuminanceSource>(&source, [](void*) {}));

        auto result = reader.read(binImage);
		if (result.isValid()) {
			return { ToString(result.format()), result.text(), "" };
		}
	}
	catch (const std::exception& e) {
		return { "", L"", e.what() };
	}
	catch (...) {
		return { "", L"", "Unknown error" };
	}
	return {};
}

EMSCRIPTEN_BINDINGS(BarcodeReader)
{
    using namespace emscripten;
    
    value_object<ReadResult>("ReadResult")
        .field("format", &ReadResult::format)
        .field("text", &ReadResult::text)
        .field("error", &ReadResult::error)
        ;
        
    function("readBarcodeFromPng", &readBarcodeFromPng);
}
