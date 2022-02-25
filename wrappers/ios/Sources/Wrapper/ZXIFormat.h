//
//  Header.h
//  
//
//  Created by Christian Braun on 25.03.22.
//

#import <Foundation/Foundation.h>
#ifndef ZXIFormat_h
#define ZXIFormat_h

typedef NS_ENUM(NSInteger, ZXIFormat) {
    NONE, AZTEC, CODABAR, CODE_39, CODE_93, CODE_128, DATA_BAR, DATA_BAR_EXPANDED,
    DATA_MATRIX, EAN_8, EAN_13, ITF, MAXICODE, PDF_417, QR_CODE, UPC_A, UPC_E,
    ONE_D_CODES, TWO_D_CODES, MICRO_QR_CODE, ANY
};

#endif 
