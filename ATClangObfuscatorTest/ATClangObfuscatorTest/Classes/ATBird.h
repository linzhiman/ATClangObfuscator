//
//  ATBird.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ATAnimal.h"
#import "IATBird.h"

NS_ASSUME_NONNULL_BEGIN

@interface ATBird : ATAnimal<IATBird>

/// property should NOT be obfuscated
@property (nonatomic, assign) BOOL full;

/// should be obfuscated
- (void)flyEx;

/// should be obfuscated
- (int)count;

/// should be obfuscated
- (void)setCount:(int)count;

@end

NS_ASSUME_NONNULL_END
