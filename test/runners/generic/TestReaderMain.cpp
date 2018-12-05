/*
* Copyright 2016 Nu-book Inc.
* Copyright 2017 Axel Waggershauser
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

#include "BlackboxTestRunner.h"
#include "ImageLoader.h"
#include "TestReader.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "TextUtfEncoding.h"
#include "ZXContainerAlgorithms.h"
#include "lodepng.h"

#include <iostream>
#include <set>

#if defined(__APPLE__) && defined(__GNUC__) && ! defined(__clang__)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

using namespace ZXing;
using namespace ZXing::Test;

static std::string toUtf8(const std::wstring& s)
{
	return TextUtfEncoding::ToUtf8(s);
}

class GenericImageLoader : public ImageLoader
{
public:
	virtual std::shared_ptr<LuminanceSource> load(const std::wstring& filename) const override
	{
		std::vector<unsigned char> buffer;
		unsigned width, height;
		unsigned error = lodepng::decode(buffer, width, height, toUtf8(filename));
		if (error) {
			throw std::runtime_error("Failed to read image");
		}
		return std::make_shared<GenericLuminanceSource>((int)width, (int)height, buffer.data(), width*4, 4, 0, 1, 2);
	}
};

class GenericBlackboxTestRunner : public BlackboxTestRunner
{
public:
	GenericBlackboxTestRunner(const std::wstring& pathPrefix)
		: BlackboxTestRunner(pathPrefix, std::make_shared<GenericImageLoader>())
	{
	}

	virtual std::vector<std::wstring> getImagesInDirectory(const std::wstring& dirPath) override
	{
		std::vector<std::wstring> result;
		for (fs::directory_iterator i(fs::path(pathPrefix())/dirPath); i != fs::directory_iterator(); ++i)
			if (is_regular_file(i->status()) && i->path().extension() == ".png")
				result.push_back(dirPath + L"/" + i->path().filename().generic_wstring());
		return result;
	}
};

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	std::string pathPrefix = argv[1];

	GenericBlackboxTestRunner runner(std::wstring(pathPrefix.begin(), pathPrefix.end()));

	if ( Contains(std::vector<std::string>{".png", ".jpg", ".pgm", ".gif"}, fs::path(pathPrefix).extension()) ) {
#if 0
		TestReader reader = runner.createReader(false, false, "QR_CODE");
#else
		TestReader reader = runner.createReader(true, true);
#endif
		bool isPure = getenv("IS_PURE");
		int rotation = getenv("ROTATION") ? atoi(getenv("ROTATION")) : 0;

		for (int i = 1; i < argc; ++i) {
			auto result = reader.read(fs::path(argv[i]).generic_wstring(), rotation, isPure);
			std::cout << argv[i] << ": ";
			if (result)
				std::cout << result.format << ": " << result.text << "\n";
			else
				std::cout << "FAILED\n";
		}
		return 0;
	}

	std::set<std::string> includedTests;
	for (int i = 2; i < argc; ++i) {
		if (std::strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == 't') {
			includedTests.insert(argv[i] + 2);
		}
	}

	runner.run(includedTests);
}
