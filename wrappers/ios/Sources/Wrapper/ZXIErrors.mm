// Copyright 2023 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIErrors.h"

void SetNSError(NSError *__autoreleasing _Nullable* error,
                NSInteger code,
                const char* message) {
    if (error == nil) {
        return;
    }
    NSString *errorDescription = @"Unknown C++ error";
    if (message && strlen(message) > 0) {
        try {
            errorDescription = [NSString stringWithUTF8String: message];
        } catch (NSException *exception) {
            errorDescription = @"Unknown ObjC error";
        }
    }
    NSDictionary *userInfo = @{ NSLocalizedDescriptionKey: errorDescription };
    *error = [NSError errorWithDomain:ZXIErrorDomain
                                 code:code
                             userInfo:userInfo];
}
