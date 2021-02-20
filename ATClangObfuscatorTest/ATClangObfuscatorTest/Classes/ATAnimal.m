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
- (void)obfuscate_private_method;

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
- (void)obfuscate_sleep
{
    /// should be obfuscated
    [self obfuscate_sleep_now:YES duration:100];
}

/// should be obfuscated
- (void)obfuscate_sleep_now:(BOOL)now duration:(NSInteger)duration
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (BOOL)obfuscate_sleep_at_night
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
- (void)obfuscate_private_method
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)obfuscate_private_method_ex
{
    NSLog(@"do nothing");
}

/// should be obfuscated
+ (BOOL)obfuscate_is_any_instance_live
{
    return kATAnimalLiveCount > 0;
}

@end
