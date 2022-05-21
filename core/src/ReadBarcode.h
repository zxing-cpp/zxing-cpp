/*
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "DecodeHints.h"
#include "ImageView.h"
#include "Result.h"

namespace ZXing {

/**
 * Read barcode from an ImageView
 *
 * @param buffer  view of the image data including layout and format
 * @param hints  optional DecodeHints to parameterize / speed up decoding
 * @return #Result structure
 */
Result ReadBarcode(const ImageView& buffer, const DecodeHints& hints = {});

// WARNING: this API is experimental and may change/disappear
Results ReadBarcodes(const ImageView& buffer, const DecodeHints& hints = {});

} // ZXing

