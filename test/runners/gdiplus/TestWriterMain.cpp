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

#include "qrcode/QRWriter.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "BitMatrix.h"
#include "CharacterSet.h"
#include "ImageWriter.h"
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

	auto text = L"http://www.google.com/";
	QRCode::Writer writer;
	writer.setErrorCorrectionLevel(QRCode::ErrorCorrectionLevel::Medium);
	BitMatrix result;
	writer.encode(text, 99, 99, result);
	std::ostringstream buffer;
	result.writePBM(buffer);
	
	auto image = ImageWriter::CreateImage(result);
	savePng(*image, L"R:/test.png");
}
