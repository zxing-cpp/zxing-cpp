/*
* Copyright 2017 Huy Cuong Nguyen
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

#include "BarcodeGenerator.h"
#include "BarcodeFormat.h"
#include "CharacterSetECI.h"
#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "ImageWriter.h"

#include <stdexcept>

namespace ZXing {

BarcodeGenerator::BarcodeGenerator(const std::string& format)
{
	auto barcodeFormat = BarcodeFormatFromString(format);
	if (barcodeFormat == BarcodeFormat::FORMAT_COUNT)
		throw std::invalid_argument("Unsupported format: " + format);
	_writer.reset(new MultiFormatWriter(barcodeFormat));
}

void
BarcodeGenerator::setEncoding(const std::string& encoding)
{
	_writer->setEncoding(CharacterSetECI::CharsetFromName(encoding.c_str()));
}

void
BarcodeGenerator::setMargin(int margin)
{
	_writer->setMargin(margin);
}

std::shared_ptr<Gdiplus::Bitmap>
BarcodeGenerator::generate(const std::wstring& contents, int width, int height) const
{
	return ImageWriter::CreateImage(_writer->encode(contents, width, height));
}

} // ZXing
