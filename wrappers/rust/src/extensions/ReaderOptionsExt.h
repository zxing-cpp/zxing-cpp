/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ReaderOptions.h"

#include <vector>

namespace ZXing {

class ReaderOptionsExt : public ReaderOptions
{

public:
    ReaderOptionsExt() {}

    ReaderOptionsExt& addFormat(BarcodeFormat f)
    {
        BarcodeFormats newFormats(f);
        newFormats |= this->formats();
        this->setFormats(newFormats);
        return *this;
    }

    std::vector<BarcodeFormat> allFormats() const
    {
        std::vector<BarcodeFormat> formats;
        for(auto format : this->formats())
        {
            formats.emplace_back(format);
        }
        return formats;
    }

    ReaderOptions& asOptions()
    {
        return *this;
    }
};

} // ZXing
