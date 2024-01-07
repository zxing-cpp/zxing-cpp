#pragma once

#include "BarcodeFormat.h"
#include "Content.h"
#include "Error.h"
#include "Result.h"
#include "ReadBarcode.h"

namespace ZXing {

std::string BarcodeFormatToString(BarcodeFormat format)
{
    return ToString(format);
}

std::string ContentTypeToString(ContentType ty)
{
    return ToString(ty);
}

std::string ErrorToString(const Error& e)
{
    return ToString(e);
}

const std::vector<uint8_t>& ByteArrayAsVec(const ByteArray& ba)
{
    return dynamic_cast<const std::vector<uint8_t>&>(ba);
}

}