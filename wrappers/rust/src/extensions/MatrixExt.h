#pragma once

#include "Matrix.h"

namespace ZXing
{

class MatrixExt
{

public:
    MatrixExt(int width, int height, uint8_t val = 0) : _inner(width, height, val) {}
    MatrixExt(Matrix<uint8_t> inner) : _inner(inner.copy()) {}

    const uint8_t* data() const {
        return _inner.data();
    }

    int height() const {
        return _inner.height();
    }

    int width() const {
        return _inner.width();
    }

    int size() const {
        return _inner.size();
    }

private:
    Matrix<uint8_t> _inner;

};

}