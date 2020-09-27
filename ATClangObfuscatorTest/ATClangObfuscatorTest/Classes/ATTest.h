//
//  ATTest.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ATMacro.h"

@class ATMonster;

NS_ASSUME_NONNULL_BEGIN

@interface ATTest : NSObject

AT_DECLARE_SINGLETON;

- (void)test;

- (void)setMonster:(ATMonster *)monster;

@end

NS_ASSUME_NONNULL_END
