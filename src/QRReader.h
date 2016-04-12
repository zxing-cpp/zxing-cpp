#pragma once
#include "Reader.h"

namespace ZXing {

class QRReader : public Reader
{
public:
	virtual ~QRReader();
	virtual Result decode(const BinaryBitmap& image, const DecodeHints* hints = nullptr) const override;
};

} // ZXing
