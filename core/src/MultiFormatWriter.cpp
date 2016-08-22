/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "MultiFormatWriter.h"
#include "EncodeHints.h"
#include "BarcodeFormat.h"
#include "BitMatrix.h"

namespace ZXing {

MultiFormatWriter::MultiFormatWriter(const EncodeHints& hints)
{
	
}

bool
MultiFormatWriter::encode(const std::wstring& contents, BarcodeFormat format, int width, int height, BitMatrix& result) const
{
	switch (format) {
		case BarcodeFormat::EAN_8:
			writer = new EAN8Writer();
			break;
		case BarcodeFormat::UPC_E:
			writer = new UPCEWriter();
			break;
		case BarcodeFormat::EAN_13:
			writer = new EAN13Writer();
			break;
		case BarcodeFormat::UPC_A:
			writer = new UPCAWriter();
			break;
		case BarcodeFormat::QR_CODE:
			writer = new QRCodeWriter();
			break;
		case BarcodeFormat::CODE_39:
			writer = new Code39Writer();
			break;
		case BarcodeFormat::CODE_93:
			writer = new Code93Writer();
			break;
		case BarcodeFormat::CODE_128:
			writer = new Code128Writer();
			break;
		case BarcodeFormat::ITF:
			writer = new ITFWriter();
			break;
		case BarcodeFormat::PDF_417:
			writer = new PDF417Writer();
			break;
		case BarcodeFormat::CODABAR:
			writer = new CodaBarWriter();
			break;
		case BarcodeFormat::DATA_MATRIX:
			writer = new DataMatrixWriter();
			break;
		case BarcodeFormat::AZTEC:
			writer = new AztecWriter();
			break;
	default:
		throw new IllegalArgumentException("No encoder available for format " + format);
	}
	return writer.encode(contents, format, width, height, hints);
}

} // ZXing
