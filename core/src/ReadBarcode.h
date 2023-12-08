/*
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ReaderOptions.h"
#include "ImageView.h"
#include "Result.h"

namespace ZXing {

/**
 * Read barcode from an ImageView
 *
 * @param image  view of the image data including layout and format
 * @param options  optional ReaderOptions to parameterize / speed up detection
 * @return #Result structure
 */
Result ReadBarcode(const ImageView& image, const ReaderOptions& options = {});

/**
 * Read barcodes from an ImageView
 *
 * @param image  view of the image data including layout and format
 * @param options  optional ReaderOptions to parameterize / speed up detection
 * @return #Results list of results found, may be empty
 */
Results ReadBarcodes(const ImageView& image, const ReaderOptions& options = {});

} // ZXing

