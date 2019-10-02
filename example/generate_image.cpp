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
#include "ByteMatrix.h"
#include "TextUtfEncoding.h"
#include "ZXStrConvWorkaround.h"

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
    std::cout << "Usage: " << exePath << " [-size <width>x<height>] [-margin <margin>] [-encoding <encoding>] [-ecc <level>] <format> <text> <output>" << std::endl
        << "    -size      Size of generated image" << std::endl
        << "    -margin    Margin around barcode" << std::endl
        << "    -encoding  Encoding used to encode input text" << std::endl
        << "    -ecc       Error correction level, [0-8]"
        << std::endl
        << "Supported formats are:" << std::endl
        << "    AZTEC" << std::endl
        << "    CODABAR" << std::endl
        << "    CODE_39" << std::endl
        << "    CODE_93" << std::endl
        << "    CODE_128" << std::endl
        << "    DATA_MATRIX" << std::endl
        << "    EAN_8" << std::endl
        << "    EAN_13" << std::endl
        << "    ITF" << std::endl
        << "    PDF_417" << std::endl
        << "    QR_CODE" << std::endl
        << "    UPC_A" << std::endl
        << "    UPC_E" << std::endl
        << "Formats can be lowercase letters, with or without underscore." << std::endl;
}

static std::string FormatClean(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    return str;
}

static std::string ParseFormat(std::string str)
{
    str = FormatClean(str);
    for (int i = 0; i < (int)BarcodeFormat::FORMAT_COUNT; ++i) {
        auto standardForm = ToString((BarcodeFormat)i);
        if (str == FormatClean(standardForm))
            return standardForm;
    }
    return std::string();
}

static bool ParseSize(std::string str, int* width, int* height)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
    auto xPos = str.find('x');
    if (xPos != std::string::npos) {
        *width = std::stoi(str.substr(0, xPos));
        *height = std::stoi(str.substr(xPos + 1));
        return true;
    }
    return false;
}

static bool ParseOptions(int argc, char* argv[], int* width, int* height, int* margin, int* eccLevel, std::string* format, std::string* text, std::string* filePath)
{
    int nonOptArgCount = 0;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-size") == 0) {
            if (i + 1 < argc) {
                ++i;
                if (!ParseSize(argv[i], width, height)) {
                    std::cerr << "Invalid size specification: " << argv[i] << std::endl;
                    return false;
                }
            }
            else {
                return false;
            }
        }
        else if (strcmp(argv[i], "-margin") == 0) {
            if (i + 1 < argc) {
                ++i;
                *margin = std::stoi(argv[i]);
            }
            else {
                return false;
            }
        }
        else if (strcmp(argv[i], "-ecc") == 0) {
            if (i + 1 < argc) {
                ++i;
                *eccLevel = std::stoi(argv[i]);
            }
            else {
                return false;
            }
        }
        else if (nonOptArgCount == 0) {
            *format = ParseFormat(argv[i]);
            if (format->empty()) {
                std::cerr << "Unreconigned format: " << argv[i] << std::endl;
                return false;
            }
            ++nonOptArgCount;
        }
        else if (nonOptArgCount == 1) {
            *text = argv[i];
            ++nonOptArgCount;
        }
        else if (nonOptArgCount == 2) {
            *filePath = argv[i];
            ++nonOptArgCount;
        }
        else {
            return false;
        }
    }

    return !format->empty() && !text->empty() && !filePath->empty();
}

static std::string GetExtension(const std::string& path)
{
    auto fileNameStart = path.find_last_of("/\\");
    auto fileName = fileNameStart == std::string::npos ? path : path.substr(fileNameStart + 1);
    auto extStart = fileName.find_last_of('.');
    auto ext = extStart == std::string::npos ? "" : fileName.substr(extStart + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    return ext;
}

int main(int argc, char* argv[])
{
    if (argc <= 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    int width = 100, height = 100;
    int margin = 10;
    int eccLevel = -1;
    std::string format, text, filePath;

    if (!ParseOptions(argc, argv, &width, &height, &margin, &eccLevel, &format, &text, &filePath)) {
        PrintUsage(argv[0]);
        return -1;
    }

    try {
        auto barcodeFormat = BarcodeFormatFromString(format);
        if (barcodeFormat == BarcodeFormat::FORMAT_COUNT)
            throw std::invalid_argument("Unsupported format: " + format);

        MultiFormatWriter writer(barcodeFormat);
        if (margin >= 0)
            writer.setMargin(margin);
        if (eccLevel >= 0)
            writer.setEccLevel(eccLevel);

        auto bitmap = writer.encode(TextUtfEncoding::FromUtf8(text), width, height).toByteMatrix();

        auto ext = GetExtension(filePath);
        int success = 0;
        if (ext == "" || ext == "png") {
            success = stbi_write_png(filePath.c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
        }
        else if (ext == "jpg" || ext == "jpeg") {
            success = stbi_write_jpg(filePath.c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
        }
 
        if (!success) {
            std::cerr << "Failed to write image: " << filePath << std::endl;
            return -1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
