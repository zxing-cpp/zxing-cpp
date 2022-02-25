//
//  ZXIDecodeHints.h
//  
//
//  Created by Christian Braun on 25.02.22.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIDecodeHints : NSObject
@property(nonatomic) BOOL tryHarder;
@property(nonatomic) BOOL tryRotate;
/// An array of ZXIFormat
@property(nonatomic, strong) NSArray<NSNumber*> *formats;

- (instancetype)initWithTryHarder:(BOOL)tryHarder tryRotate:(BOOL)tryRotate formats:(NSArray<NSNumber*>*)formats;
@end

NS_ASSUME_NONNULL_END
