//
//  ATMonster+Action.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATMonster.h"

NS_ASSUME_NONNULL_BEGIN

@interface ATMonster (Action)

/// property should NOT be obfuscated
@property (nonatomic, assign) BOOL alive;

/// should be obfuscated
- (void)doAction;

/// should be obfuscated
+ (void)doClassAction;

@end

@protocol IATActionEx <NSObject>

/// should NOT be obfuscated : ATMonster (ActionEx) conforms to IATActionEx and it has property named dead
- (BOOL)dead;

/// should be obfuscated
- (void)doActionEx;

/// property should NOT be obfuscated
- (NSString *)otherName;

@end

@interface ATMonster (ActionEx)<IATActionEx>

/// property should NOT be obfuscated
@property (nonatomic, assign) BOOL dead;

@end

NS_ASSUME_NONNULL_END
