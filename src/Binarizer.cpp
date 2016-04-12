#include "Binarizer.h"
#include "LuminanceSource.h"

namespace ZXing {

Binarizer::~Binarizer()
{
}

int
Binarizer::width() const
{
	return _source->width();
}

int
Binarizer::height() const
{
	return _source->height();
}


}