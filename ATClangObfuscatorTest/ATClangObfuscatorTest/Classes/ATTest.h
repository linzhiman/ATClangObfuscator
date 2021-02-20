//
//  ATTest.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ATMacro.h"
#import "ATDefine.h"

@class ATMonster;

NS_ASSUME_NONNULL_BEGIN

@interface ATTest : NSObject

AT_DECLARE_SINGLETON;
AT_DECLARE_CREATE_OBJ;

/// should be obfuscated
- (void)obfuscate_test;

/// should be obfuscated
- (void)setObfuscate_monster:(ATMonster *)monster;

@end

NS_ASSUME_NONNULL_END
