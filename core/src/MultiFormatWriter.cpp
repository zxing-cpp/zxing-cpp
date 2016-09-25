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
#include "BarcodeFormat.h"
#include "EncodeStatus.h"

#include "oned/ODCodabarWriter.h"
#include "oned/ODCode39Writer.h"
#include "oned/ODCode93Writer.h"
#include "oned/ODCode128Writer.h"
#include "oned/ODEAN8Writer.h"
#include "oned/ODEAN13Writer.h"
#include "oned/ODITFWriter.h"
#include "oned/ODUPCEWriter.h"
#include "oned/ODUPCAWriter.h"
#include "qrcode/QRWriter.h"
#include "pdf417/PDFWriter.h"

namespace ZXing {

EncodeStatus
MultiFormatWriter::Encode(const std::wstring& contents, BarcodeFormat format, int width, int height, const EncodeHints& hints, BitMatrix& output)
{
	switch (format) {
		case BarcodeFormat::EAN_8:
			return OneD::EAN8Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::UPC_E:
			return OneD::UPCEWriter::Encode(contents, width, height, hints, output);
		case BarcodeFormat::EAN_13:
			return OneD::EAN13Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::UPC_A:
			return OneD::UPCAWriter::Encode(contents, width, height, hints, output);
		case BarcodeFormat::QR_CODE:
			return QRCode::Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::CODE_39:
			return OneD::Code39Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::CODE_93:
			return OneD::Code93Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::CODE_128:
			return OneD::Code128Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::ITF:
			return OneD::ITFWriter::Encode(contents, width, height, hints, output);
		case BarcodeFormat::PDF_417:
			return Pdf417::Writer::Encode(contents, width, height, hints, output);
		case BarcodeFormat::CODABAR:
			return OneD::CodabarWriter::Encode(contents, width, height, hints, output);
		//case BarcodeFormat::DATA_MATRIX:
		//	writer = new DataMatrixWriter();
		//	break;
		//case BarcodeFormat::AZTEC:
		//	writer = new AztecWriter();
		//	break;
	default:
		return EncodeStatus::WithError(std::string("No encoder available for format ") + ToString(format));
	}
}

} // ZXing
