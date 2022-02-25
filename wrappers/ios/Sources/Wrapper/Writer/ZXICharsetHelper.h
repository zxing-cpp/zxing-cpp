//
//  ZXICharsetHelper.h
//  
//
//  Created by Hendrik von Prince on 25.03.22.
//

#import <Foundation/Foundation.h>
#import "ZXing/CharacterSet.h"
#import "ZXICharset.h"

NS_ASSUME_NONNULL_BEGIN

ZXICharset ZXICharsetFromCharset(ZXing::CharacterSet charset);
ZXing::CharacterSet CharsetFromZXICharset(ZXICharset charset);

NS_ASSUME_NONNULL_END
