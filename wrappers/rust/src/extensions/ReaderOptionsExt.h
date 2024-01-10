/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ReaderOptions.h"
#include "BitHacks.h"

#include <type_traits>

namespace ZXing {

class ReaderOptionsExt : public ReaderOptions
{

public:
    ReaderOptionsExt() {}

    ReaderOptionsExt& setFormats(int flags)
    {
        BarcodeFormats formats;
        for (int32_t pos = 0; pos <= BitHacks::HighestBitSet(flags); ++pos)
        {
            auto bit = flags & (1 << pos);
            formats |= static_cast<BarcodeFormat>(bit);
        }
        ReaderOptions::setFormats(formats);
        return *this;
    }

    ReaderOptionsExt& tryHarder(bool try_harder)
    {
        this->setTryHarder(try_harder);
        return *this;
    }

    ReaderOptionsExt& tryRotate(bool try_rotate)
    {
        this->setTryRotate(try_rotate);
        return *this;
    }

    ReaderOptionsExt& tryInvert(bool try_invert)
    {
        this->setTryInvert(try_invert);
        return *this;
    }
    
    ReaderOptionsExt& tryDownscale(bool try_downscale)
    {
        this->setTryDownscale(try_downscale);
        return *this;
    }
    
    ReaderOptionsExt& pure(bool pure)
    {
        this->setTryInvert(pure);
        return *this;
    }

    ReaderOptionsExt& returnErrors(bool return_errors)
    {
        this->setReturnErrors(return_errors);
        return *this;
    }

    ReaderOptionsExt& binarizer(Binarizer binarizer)
    {
        this->setBinarizer(binarizer);
        return *this;
    }

    ReaderOptionsExt& eanAddOnSymbol(EanAddOnSymbol ean_add_on_symbol)
    {
        this->setEanAddOnSymbol(ean_add_on_symbol);
        return *this;
    }

    ReaderOptionsExt& textMode(TextMode text_mode)
    {
        this->setTextMode(text_mode);
        return *this;
    }
};

} // ZXing
