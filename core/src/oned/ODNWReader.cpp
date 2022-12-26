//
// Created by yedai on 2022/12/16.
//

#include "ODNWReader.h"

#include <iostream>

namespace ZXing {
namespace OneD {

Result ODNWReader::decodePattern(int rowNumber, PatternView &next, std::unique_ptr<DecodingState> &state) const {
	auto size = 10;
	std::string txt;
	txt.reserve(20);
	next = next.subView(0, size);
	int xStart = next.pixelsInFront();
	BarAndSpaceI threshold;

	while(next.isValid()) {
		threshold = NarrowWideThreshold(next);
		if (!threshold.isValid())
			break;
		for (int i = 0; i < size; ++i) {
			txt.push_back(next[i] > threshold[i] ? 'W' : 'N');
			/*
char bit = i % 2 ? '0' : '1';
			if(next[i] > threshold[i]) {
				txt.push_back(bit);
				txt.push_back(bit);
			}
			else{
				txt.push_back(bit);
			}
			 */
		}
		next.skipSymbol();
	}


	for(int i = size; i > 0; i--) {
		if(next.isValid(i)) {
			size = i;
			break;
		}
	}
	next.subView(0, size);
	for (int i = 0; i < size; ++i) {
		if(i % 2 && i == size - 1) {
			break;
		}
		txt.push_back(next[i] > threshold[i] ? 'W' : 'N');
		/*
char bit = i % 2 ? '0' : '1';
		if(next[i] > threshold[i]) {
			txt.push_back(bit);
			txt.push_back(bit);
		}
		else{
			txt.push_back(bit);
		}
		 */
	}


	int xStop = next.pixelsTillEnd();
	SymbologyIdentifier symbologyIdentifier = {0}; // No check character validation
	Error error;

	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::NWCode, symbologyIdentifier, error);

};

} // namespace OneD
} // namespace ZXing