/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "DecodeHints.h"

namespace ZXing {
	BinarizerV2 UpgradeBinarizer(Binarizer binarizer){
		switch(binarizer){
			case Binarizer::BoolCast:
			return BinarizerV2::BoolCast;
			case Binarizer::FixedThreshold:
			return BinarizerV2::FixedThreshold;
			case Binarizer::GlobalHistogram:
			return BinarizerV2::GlobalHistogram;
			case Binarizer::LocalAverage:
			return BinarizerV2::LocalAverage;
		}

		return BinarizerV2::None;
	}
} // ZXing
