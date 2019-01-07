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

#include "BarcodeFormat.h"
#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "CharacterSetECI.h"
#include "lodepng.h"

#include <string>
#include <memory>
#include <emscripten/bind.h>
#include <emscripten/val.h>

class WriteResult
{
    std::shared_ptr<std::vector<unsigned char>> _pixmapBuffer;
    std::string _error;
    
public:
    WriteResult(const std::shared_ptr<std::vector<unsigned char>>& pixmap) : _pixmapBuffer(pixmap) {}
    WriteResult(const std::string& error) : _error(error) {}
    
    std::string error() const {
        return _error;
    }
    
    emscripten::val pixmapBytes() const {
        if (_pixmapBuffer != nullptr)
            return emscripten::val(emscripten::typed_memory_view(_pixmapBuffer->size(), _pixmapBuffer->data()));
        else
            return emscripten::val::null();
    }
};

WriteResult generateBarcode(std::wstring text, std::string format, std::string encoding, int margin, int width, int height, int eccLevel)
{
    using namespace ZXing;
	try {
		auto barcodeFormat = BarcodeFormatFromString(format);
		if (barcodeFormat == BarcodeFormat::FORMAT_COUNT)
			throw std::invalid_argument("Unsupported format: " + format);
		
		MultiFormatWriter writer(barcodeFormat);
		if (margin >= 0)
			writer.setMargin(margin);
        
        CharacterSet charset = CharacterSetECI::CharsetFromName(encoding.c_str());
        if (charset != CharacterSet::Unknown) {
            writer.setEncoding(charset);
        }

        if (eccLevel >= 0 && eccLevel <= 8) {
            writer.setEccLevel(eccLevel);
        }

		auto matrix = writer.encode(text, width, height);

		std::vector<unsigned char> buffer(matrix.width() * matrix.height(), '\0');
		unsigned char black = 0;
		unsigned char white = 255;
		for (int y = 0; y < matrix.height(); ++y) {
			for (int x = 0; x < matrix.width(); ++x) {
				buffer[y * matrix.width() + x] = matrix.get(x, y) ? black : white;
			}
		}

		auto outputBuffer = std::make_shared<std::vector<unsigned char>>();
		unsigned error = lodepng::encode(*outputBuffer, buffer, matrix.width(), matrix.height(), LCT_GREY);
		if (error) {
			return WriteResult(lodepng_error_text(error));
		}

        return WriteResult(outputBuffer);
	}
	catch (const std::exception& e) {
        return WriteResult(std::string(e.what()));
	}
    catch (...) {
        return WriteResult(std::string("Unknown error"));
    }
}

EMSCRIPTEN_BINDINGS(BarcodeWriter)
{
    using namespace emscripten;
    
    class_<WriteResult>("WriteResult")
        .property("pixmap", &WriteResult::pixmapBytes)
        .property("error", &WriteResult::error)
        ;
        
    function("generateBarcode", &generateBarcode);
}
