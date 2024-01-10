#pragma once

#include "BarcodeFormat.h"
#include "Content.h"
#include "CharacterSet.h"
#include "Error.h"
#include "rust/cxx.h"
#include "ReadBarcode.h"
#include "ReaderOptionsExt.h"

#include <memory>
#include <vector>
#include <string>

namespace ZXing {

std::unique_ptr<ImageView> newImageView(const uint8_t* data, int width, int height, ImageFormat format, int rowStride, int pixStride)
{
    return std::unique_ptr<ImageView>(new ImageView(data, width, height, format, rowStride, pixStride));
}

std::unique_ptr<ReaderOptionsExt> newReaderOptions()
{
    return std::unique_ptr<ReaderOptionsExt>(new ReaderOptionsExt());
}

rust::String barcodeFormatToString(int format)
{
    return ToString(static_cast<BarcodeFormat>(format));
}

rust::String contentTypeToString(ContentType ty)
{
    return ToString(ty);
}

rust::String errorToString(const Error& e)
{
    return ToString(e);
}

rust::String characterSetToString(CharacterSet cs)
{
    return ToString(cs);
}

const std::vector<uint8_t>& byteArrayAsVec(const ByteArray& ba)
{
    return dynamic_cast<const std::vector<uint8_t>&>(ba);
}

int formatOfResult(const Result& res)
{
    return static_cast<int>(res.format());
}

rust::String textOfResult(const Result& res)
{
    return res.text();
}

rust::String ecLevelOfResult(const Result& res)
{
    return res.ecLevel();
}

rust::String symbologyIdentifierOfResult(const Result& res)
{
    return res.symbologyIdentifier();
}

std::unique_ptr<std::vector<Result>> readBarcodes(const ImageView& image, const ReaderOptionsExt& read_options) noexcept(false)
{
    try {
        return std::make_unique<std::vector<Result>>(ReadBarcodes(image, read_options));
    } catch (std::exception e) {
        throw e;
    }
}

}