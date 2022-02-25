//
//  ZXIErrors.h
//  
//
//  Created by Christian Braun on 25.02.22.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#define ZXIErrorDomain @"ZXIErrorDomain"

typedef NS_ENUM(NSInteger, ZXIReaderError) {
    ZXINotFoundError = 1000,
    ZXIChecksumError,
    ZXIFormatError,
    ZXIWriterError,
};

NS_ASSUME_NONNULL_END
