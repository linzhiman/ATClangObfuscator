//
//  ATMonster+Action.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATMonster+Action.h"

@protocol IATActionBase <NSObject>

/// should be obfuscated
- (NSString *)obfuscate_base;

@end

@interface ATMonster (ActionPrivate)<IATActionBase>

@end

@implementation ATMonster (ActionPrivate)

/// should be obfuscated
- (NSString *)obfuscate_base;
{
    return nil;
}

@end

@implementation ATMonster (Action)

/// should be obfuscated
- (void)obfuscate_do_action
{
    NSLog(@"do nothing");
}

/// should be obfuscated
+ (void)obfuscate_do_class_action
{
    NSLog(@"do nothing");
}

/// property should NOT be obfuscated
- (BOOL)alive
{
    return YES;
}

/// property should NOT be obfuscated
- (void)setAlive:(BOOL)alive
{
    NSLog(@"do nothing");
}

@end

@implementation ATMonster (ActionEx)

/// property should NOT be obfuscated
- (BOOL)dead
{
    return YES;
}

/// property should NOT be obfuscated
- (void)setDead:(BOOL)dead
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)obfuscate_do_action_ex
{
    NSLog(@"do nothing");
}

@end
