//
//  ATMonster+Action.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATMonster+Action.h"

@protocol IATActionBaseInner <NSObject>

- (NSString *)inner;

@end

@interface ATMonster (ActionInner)<IATActionBaseInner>

@end

@implementation ATMonster (ActionInner)

- (NSString *)inner;
{
    return nil;
}

@end

@implementation ATMonster (Action)

- (void)doAction
{
    ;;
}

+ (void)doClassAction
{
    ;;
}

- (BOOL)alive
{
    return YES;
}

- (void)setAlive:(BOOL)alive
{
    ;;
}

@end

@implementation ATMonster (Action2)

- (NSString *)actionBaseName;
{
    return [self inner];
}

@end
