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

#include <string>
#include <memory>
#include <exception>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

class ImageData
{
public:
	uint8_t* const buffer;
	const int length;

	ImageData(uint8_t* buf, int len) : buffer(buf), length(len) {}
	~ImageData() { STBIW_FREE(buffer); }
};

class WriteResult
{
	std::shared_ptr<ImageData> _image;
	std::string _error;

public:
	WriteResult(const std::shared_ptr<ImageData>& image) : _image(image) {}
	WriteResult(std::string error) : _error(std::move(error)) {}

	std::string error() const { return _error; }

	emscripten::val image() const
	{
		if (_image != nullptr)
			return emscripten::val(emscripten::typed_memory_view(_image->length, _image->buffer));
		else
			return emscripten::val::null();
	}
};

WriteResult generateBarcode(std::wstring text, std::string format, std::string encoding, int margin, int width, int height, int eccLevel)
{
	using namespace ZXing;
	try {
		auto barcodeFormat = BarcodeFormatFromString(format);
		if (barcodeFormat == BarcodeFormat::None)
			return {"Unsupported format: " + format};

		MultiFormatWriter writer(barcodeFormat);
		if (margin >= 0)
			writer.setMargin(margin);

		CharacterSet charset = CharacterSetECI::CharsetFromName(encoding.c_str());
		if (charset != CharacterSet::Unknown)
			writer.setEncoding(charset);

		if (eccLevel >= 0 && eccLevel <= 8)
			writer.setEccLevel(eccLevel);

		auto buffer = ToMatrix<uint8_t>(writer.encode(text, width, height));

		int len;
		uint8_t* bytes = stbi_write_png_to_mem(buffer.data(), 0, buffer.width(), buffer.height(), 1, &len);
		if (bytes == nullptr)
			return {"Unknown error"};

		return {std::make_shared<ImageData>(bytes, len)};
	} catch (const std::exception& e) {
		return {e.what()};
	} catch (...) {
		return {"Unknown error"};
	}
}

EMSCRIPTEN_BINDINGS(BarcodeWriter)
{
	using namespace emscripten;

	class_<WriteResult>("WriteResult")
	    .property("image", &WriteResult::image)
	    .property("error", &WriteResult::error)
	    ;

	function("generateBarcode", &generateBarcode);
}
