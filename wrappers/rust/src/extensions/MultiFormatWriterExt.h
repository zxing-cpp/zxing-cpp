#pragma once

#include "MultiFormatWriter.h"
#include "MatrixExt.h"
#include "BitMatrix.h"

namespace ZXing
{

class MultiFormatWriterExt : public MultiFormatWriter
{

public:
    MultiFormatWriterExt(BarcodeFormat f) : MultiFormatWriter(f) {}

    MultiFormatWriter& asBase()
    {
        return *this;
    }

    MatrixExt encodeToMatrix(const std::string& contents, int width, int height) const
    {
        auto bit_matrix = this->encode(contents, width, height);
        return MatrixExt(ToMatrix<uint8_t>(bit_matrix));
    }

};

}