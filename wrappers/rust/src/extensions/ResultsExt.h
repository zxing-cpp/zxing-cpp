#pragma once

#include "Result.h"

namespace ZXing {

class ResultsExt : public std::vector<Result>
{
public:
    ResultsExt(std::vector<Result> results): std::vector<Result>(results) {}

    const Result& at(int i) const
    {
        return std::vector<Result>::at(i);
    }

    int size() const
    {
        return std::vector<Result>::size();
    }
};

}