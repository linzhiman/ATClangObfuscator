//
//  ATAnimal.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATAnimal.h"

@interface ATAnimal()

@property (nonatomic, assign) int innerA;
@property (nonatomic, assign) int innerB;
@property (nonatomic, assign) int innerC;

- (void)innerFunction;

@end

@implementation ATAnimal

- (id)init
{
    self = [super init];
    if (self) {
        ;;
    }
    return self;
}

- (void)dealloc
{
    ;;
}

+ (NSString *)description
{
    return nil;
}

- (void)sleep
{
    [self sleepNow:YES duration:100];
}

- (void)sleepNow:(BOOL)now duration:(NSInteger)duration
{
    ;;
}

- (BOOL)sleepAtNight
{
    return YES;
}

- (int)type
{
    return 0;
}

- (void)setType:(int)type
{
    ;;
}

- (int)age
{
    return _age;
}

- (void)setArea:(NSString *)area
{
    _area = area;
}

- (NSString *)className
{
    return nil;
}

- (void)setSubClassName:(NSString *)subClassName
{
    _subClassName = subClassName;
}

- (int)innerB
{
    return _innerB;
}

- (void)setInnerC:(int)innerC
{
    _innerC = innerC;
}

- (void)innerFunction
{
    NSLog(@"Function %@", @"innerFunction");
}

- (void)innerFunction2
{
    NSLog(@"Function %@", @"innerFunction2");
}

+ (BOOL)isAnyInstanceLive
{
    return YES;
}

@end
