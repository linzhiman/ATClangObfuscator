//
//  ATAnimal.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATAnimal.h"

static int kATAnimalLiveCount = 0;

@interface ATAnimal()

/// all property should NOT be obfuscated

@property (nonatomic, assign) int privatePA;
@property (nonatomic, assign) int privatePB;
@property (nonatomic, assign) int privatePC;

/// should be obfuscated
- (void)privateMethod;

@end

@implementation ATAnimal

/// sys method should NOT be obfuscated
- (id)init
{
    self = [super init];
    if (self) {
        kATAnimalLiveCount++;
    }
    return self;
}

/// sys method should NOT be obfuscated
- (void)dealloc
{
    kATAnimalLiveCount--;
}

/// sys method should NOT be obfuscated
- (NSString *)description
{
    /// property should NOT be obfuscated
    return self.name;
}

/// getter/setter should NOT be obfuscated
- (int)type
{
    return 0;
}

/// getter/setter should NOT be obfuscated
- (void)setType:(int)type
{
    NSLog(@"do nothing");
}

/// getter/setter should NOT be obfuscated
- (int)age
{
    return _age;
}

/// getter/setter should NOT be obfuscated
- (void)setArea:(NSString *)area
{
    _area = area;
}

/// getter/setter should NOT be obfuscated
- (NSString *)className
{
    return _className;
}

/// getter/setter should NOT be obfuscated
- (void)setSubClassName:(NSString *)subClassName
{
    _subClassName = subClassName;
}

/// should be obfuscated
- (void)sleep
{
    /// should be obfuscated
    [self sleepNow:YES duration:100];
}

/// should be obfuscated
- (void)sleepNow:(BOOL)now duration:(NSInteger)duration
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (BOOL)sleepAtNight
{
    return YES;
}

/// getter/setter should NOT be obfuscated
- (int)privatePB
{
    return _privatePB;
}

/// getter/setter should NOT be obfuscated
- (void)setPrivatePC:(int)privatePC
{
    _privatePC = privatePC;
}

/// should be obfuscated
- (void)privateMethod
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)privateMethodEx
{
    NSLog(@"do nothing");
}

/// should be obfuscated
+ (BOOL)isAnyInstanceLive
{
    return kATAnimalLiveCount > 0;
}

@end
