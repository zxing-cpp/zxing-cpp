// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXICharsetHelper.h"

ZXICharset ZXICharsetFromCharset(ZXing::CharacterSet charset) {
    switch(charset) {
        case ZXing::CharacterSet::Unknown:
            return Unknown;
        case ZXing::CharacterSet::ASCII:
            return ASCII;
        case ZXing::CharacterSet::ISO8859_1:
            return ISO8859_1;
        case ZXing::CharacterSet::ISO8859_2:
            return ISO8859_2;
        case ZXing::CharacterSet::ISO8859_3:
            return ISO8859_3;
        case ZXing::CharacterSet::ISO8859_4:
            return ISO8859_4;
        case ZXing::CharacterSet::ISO8859_5:
            return ISO8859_5;
        case ZXing::CharacterSet::ISO8859_6:
            return ISO8859_6;
        case ZXing::CharacterSet::ISO8859_7:
            return ISO8859_7;
        case ZXing::CharacterSet::ISO8859_8:
            return ISO8859_8;
        case ZXing::CharacterSet::ISO8859_9:
            return ISO8859_9;
        case ZXing::CharacterSet::ISO8859_10:
            return ISO8859_10;
        case ZXing::CharacterSet::ISO8859_11:
            return ISO8859_11;
        case ZXing::CharacterSet::ISO8859_13:
            return ISO8859_13;
        case ZXing::CharacterSet::ISO8859_14:
            return ISO8859_14;
        case ZXing::CharacterSet::ISO8859_15:
            return ISO8859_15;
        case ZXing::CharacterSet::ISO8859_16:
            return ISO8859_16;
        case ZXing::CharacterSet::Cp437:
            return Cp437;
        case ZXing::CharacterSet::Cp1250:
            return Cp1250;
        case ZXing::CharacterSet::Cp1251:
            return Cp1251;
        case ZXing::CharacterSet::Cp1252:
            return Cp1252;
        case ZXing::CharacterSet::Cp1256:
            return Cp1256;
        case ZXing::CharacterSet::Shift_JIS:
            return Shift_JIS;
        case ZXing::CharacterSet::Big5:
            return Big5;
        case ZXing::CharacterSet::GB2312:
            return GB2312;
        case ZXing::CharacterSet::GB18030:
            return GB18030;
        case ZXing::CharacterSet::EUC_JP:
            return EUC_JP;
        case ZXing::CharacterSet::EUC_KR:
            return EUC_KR;
        case ZXing::CharacterSet::UnicodeBig:
            return UnicodeBig;
        case ZXing::CharacterSet::UTF8:
            return UTF8;
        case ZXing::CharacterSet::BINARY:
            return BINARY;
        case ZXing::CharacterSet::CharsetCount:
            return Unknown;
    }
    NSLog(@"ZXIWrapper: Received invalid CharacterSet, returning Unknown");
    return Unknown;
}

ZXing::CharacterSet CharsetFromZXICharset(ZXICharset charset) {
    switch(charset) {
        case Unknown:
            return ZXing::CharacterSet::Unknown;
        case ASCII:
            return ZXing::CharacterSet::ASCII;
        case ISO8859_1:
            return ZXing::CharacterSet::ISO8859_1;
        case ISO8859_2:
            return ZXing::CharacterSet::ISO8859_2;
        case ISO8859_3:
            return ZXing::CharacterSet::ISO8859_3;
        case ISO8859_4:
            return ZXing::CharacterSet::ISO8859_4;
        case ISO8859_5:
            return ZXing::CharacterSet::ISO8859_5;
        case ISO8859_6:
            return ZXing::CharacterSet::ISO8859_6;
        case ISO8859_7:
            return ZXing::CharacterSet::ISO8859_7;
        case ISO8859_8:
            return ZXing::CharacterSet::ISO8859_8;
        case ISO8859_9:
            return ZXing::CharacterSet::ISO8859_9;
        case ISO8859_10:
            return ZXing::CharacterSet::ISO8859_10;
        case ISO8859_11:
            return ZXing::CharacterSet::ISO8859_11;
        case ISO8859_13:
            return ZXing::CharacterSet::ISO8859_13;
        case ISO8859_14:
            return ZXing::CharacterSet::ISO8859_14;
        case ISO8859_15:
            return ZXing::CharacterSet::ISO8859_15;
        case ISO8859_16:
            return ZXing::CharacterSet::ISO8859_16;
        case Cp437:
            return ZXing::CharacterSet::Cp437;
        case Cp1250:
            return ZXing::CharacterSet::Cp1250;
        case Cp1251:
            return ZXing::CharacterSet::Cp1251;
        case Cp1252:
            return ZXing::CharacterSet::Cp1252;
        case Cp1256:
            return ZXing::CharacterSet::Cp1256;
        case Shift_JIS:
            return ZXing::CharacterSet::Shift_JIS;
        case Big5:
            return ZXing::CharacterSet::Big5;
        case GB2312:
            return ZXing::CharacterSet::GB2312;
        case GB18030:
            return ZXing::CharacterSet::GB18030;
        case EUC_JP:
            return ZXing::CharacterSet::EUC_JP;
        case EUC_KR:
            return ZXing::CharacterSet::EUC_KR;
        case UnicodeBig:
            return ZXing::CharacterSet::UnicodeBig;
        case UTF8:
            return ZXing::CharacterSet::UTF8;
        case BINARY:
            return ZXing::CharacterSet::BINARY;
    }
    NSLog(@"ZXIWrapper: Received invalid ZXIFormat, returning Unknown");
    return ZXing::CharacterSet::Unknown;
}
