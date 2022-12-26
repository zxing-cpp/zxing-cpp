//
// Created by yedai on 2022/12/16.
//

#ifndef ZXING_ODNWREADER_H
#define ZXING_ODNWREADER_H
#include "ODRowReader.h"
namespace ZXing {

namespace OneD {

class ODNWReader : public RowReader
{
public:
	using RowReader::RowReader;
	Result decodePattern(int rowNumber, PatternView &next, std::unique_ptr<DecodingState> &state) const override;
};

} // namespace OneD
} // namespace ZXing

#endif // ZXING_ODNWREADER_H
