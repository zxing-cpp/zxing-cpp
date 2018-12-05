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

#include "BlackboxTestRunner.h"
#include "ImageReader.h"
#include "ImageLoader.h"
#include "GdiplusInit.h"

using namespace ZXing;
using namespace ZXing::Test;

static std::wstring BuildPath(const std::wstring& dir, const std::wstring& name)
{
	if (dir.empty()) {
		return name;
	}
	if (name.empty()) {
		return dir;
	}
	if (dir.back() == '/' || name.front() == '/') {
		return dir + name;
	}
	return dir + L"/" + name;
}

static void FixeBitmapFormat(Gdiplus::Bitmap& bitmap)
{
	switch (bitmap.GetPixelFormat()) {
	case PixelFormat24bppRGB:
	case PixelFormat32bppARGB:
	case PixelFormat32bppRGB:
		break;
	default:
		if (bitmap.ConvertFormat(PixelFormat24bppRGB, Gdiplus::DitherTypeNone, Gdiplus::PaletteTypeCustom, nullptr, 0) != Gdiplus::Ok) {
			throw std::runtime_error("Cannot convert bitmap");
		}
	}
}

class GdiImageLoader : public ImageLoader
{
public:
	virtual std::shared_ptr<LuminanceSource> load(const std::wstring& filename) const override
	{
		Gdiplus::Bitmap bitmap(filename.c_str());
		FixeBitmapFormat(bitmap);
		return ImageReader::Read(bitmap);
	}
};

class GdiBlackboxTestRunner : public BlackboxTestRunner
{
public:
	GdiBlackboxTestRunner(const std::wstring& pathPrefix)
		: BlackboxTestRunner(pathPrefix, std::make_shared<GdiImageLoader>())
	{
	}

	virtual std::vector<std::wstring> getImagesInDirectory(const std::wstring& dirPath) override
	{
		std::vector<std::wstring> result;
		WIN32_FIND_DATAW data;
		HANDLE hFind = FindFirstFileW(BuildPath(pathPrefix(), BuildPath(dirPath, L"*.png")).c_str(), &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				result.push_back(BuildPath(dirPath, data.cFileName));
			} while (FindNextFileW(hFind, &data));
			FindClose(hFind);
		}
		return result;
	}
};

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	ZXing::GdiplusInit gdiplusInit;

	std::string pathPrefix = argv[1];
	GdiBlackboxTestRunner runner(std::wstring(pathPrefix.begin(), pathPrefix.end()));

	std::set<std::string> includedTests;
	for (int i = 2; i < argc; ++i) {
		if (std::strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == 't') {
			includedTests.insert(argv[i] + 2);
		}
	}

	runner.run(includedTests);
}
