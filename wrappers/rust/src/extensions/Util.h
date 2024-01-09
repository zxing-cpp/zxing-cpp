#pragma once

#include "BarcodeFormat.h"
#include "Content.h"
#include "CharacterSet.h"
#include "Error.h"
#include "ReadBarcode.h"
#include "ReaderOptionsExt.h"
#include "ResultsExt.h"

#include <vector>
#include <string>

namespace ZXing {

std::string barcodeFormatToString(int format)
{
    return ToString(static_cast<BarcodeFormat>(format));
}

std::string contentTypeToString(ContentType ty)
{
    return ToString(ty);
}

std::string errorToString(const Error& e)
{
    return ToString(e);
}

std::string characterSetToString(CharacterSet cs)
{
    return ToString(cs);
}

const std::vector<uint8_t>& byteArrayAsVec(const ByteArray& ba)
{
    return dynamic_cast<const std::vector<uint8_t>&>(ba);
}

ResultsExt readBarcodes(const ImageView& image, const ReaderOptionsExt& read_options)
{
    return ReadBarcodes(image, read_options);
}

}