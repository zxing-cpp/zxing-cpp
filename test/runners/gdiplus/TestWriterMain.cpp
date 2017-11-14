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

#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <vector>

#include "qrcode/QRWriter.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "BarcodeGenerator.h"
#include "GdiplusInit.h"

#include <sstream>

using namespace ZXing;

//static std::wstring LoadSample()

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void savePng(Gdiplus::Bitmap& bitmap, const std::wstring& filePath)
{
	using namespace Gdiplus;

	CLSID pngClsid;
	int result = GetEncoderClsid(L"image/png", &pngClsid);
	if (result == -1)
		throw std::runtime_error("GetEncoderClsid");
	
	auto saveResult = bitmap.Save(filePath.c_str(), &pngClsid, NULL);
	if (saveResult != Ok)
		throw std::runtime_error("Cannot save to PNG");
}

int main(int argc, char** argv)
{
	GdiplusInit gdiplusinit;

	std::wstring text = L"http://www.google.com/";
	for (auto format : {
		"AZTEC",
		"DATA_MATRIX",
		"PDF_417",
		"QR_CODE" })
	{
		BarcodeGenerator generator(format);
		auto bitmap = generator.generate(text, 199, 199);
		savePng(*bitmap, std::wstring(format, format + strlen(format)) + std::wstring(L"_out.png"));
	}

	text = L"012345678901234567890123456789";
	typedef std::vector<std::pair<const char*, size_t>> FormatSpecs;
	for (auto spec : FormatSpecs({
		{"CODABAR", 0},
		{"CODE_39", 0},
		{"CODE_93", 0},
		{"CODE_128", 0},
		{"EAN_8", 7},
		{"EAN_13", 12},
		{"ITF", 0},
		{"UPC_A", 11 },
		{"UPC_E", 7 } }))
	{
		auto format = spec.first;
		BarcodeGenerator generator(format);
		generator.setMargin(20);
		auto input = spec.second > 0 ? text.substr(0, spec.second) : text;
		auto bitmap = generator.generate(input, 100, 100);
		savePng(*bitmap, std::wstring(format, format + strlen(format)) + std::wstring(L"_out.png"));
	}
}
