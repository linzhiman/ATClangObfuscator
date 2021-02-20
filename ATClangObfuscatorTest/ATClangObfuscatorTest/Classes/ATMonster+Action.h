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
- (void)obfuscate_do_action;

/// should be obfuscated
+ (void)obfuscate_do_class_action;

@end

@protocol IATActionEx <NSObject>

/// should NOT be obfuscated : ATMonster (ActionEx) conforms to IATActionEx and it has property named dead
- (BOOL)dead;

/// should be obfuscated
- (void)obfuscate_do_action_ex;

@end

@interface ATMonster (ActionEx)<IATActionEx>

/// property should NOT be obfuscated
@property (nonatomic, assign) BOOL dead;

@end

NS_ASSUME_NONNULL_END
