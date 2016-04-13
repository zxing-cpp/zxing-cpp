#pragma once
#include "Reader.h"

namespace ZXing {
namespace QRCode {

class Reader : public ZXing::Reader
{
public:
	virtual ~Reader();
	virtual Result decode(const BinaryBitmap& image, const DecodeHints* hints = nullptr) const override;
};

} // QRCode
} // ZXing
