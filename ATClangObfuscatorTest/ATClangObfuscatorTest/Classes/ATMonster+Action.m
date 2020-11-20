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
- (NSString *)base;

@end

@interface ATMonster (ActionPrivate)<IATActionBase>

@end

@implementation ATMonster (ActionPrivate)

/// should be obfuscated
- (NSString *)base;
{
    return nil;
}

@end

@implementation ATMonster (Action)

/// should be obfuscated
- (void)doAction
{
    NSLog(@"do nothing");
}

/// should be obfuscated
+ (void)doClassAction
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

/// should be obfuscated
- (void)doActionEx
{
    NSLog(@"do nothing");
}

@end
