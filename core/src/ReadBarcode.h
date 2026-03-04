/*
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ReaderOptions.h"
#include "ImageView.h"
#include "Barcode.h"

namespace ZXing {

/**
 * Read barcode from an ImageView
 *
 * @param image  view of the image data including layout and format
 * @param options  optional ReaderOptions to parameterize / speed up detection
 * @return Barcode found, if any, otherwise a Barcode with empty content and format ZXing::BarcodeFormat::None
 */
Barcode ReadBarcode(const ImageView& image, const ReaderOptions& options = {});

/**
 * Read barcodes from an ImageView
 *
 * @param image  view of the image data including layout and format
 * @param options  optional ReaderOptions to parameterize / speed up detection
 * @return List of Barcode found, may be empty
 */
Barcodes ReadBarcodes(const ImageView& image, const ReaderOptions& options = {});

} // ZXing

