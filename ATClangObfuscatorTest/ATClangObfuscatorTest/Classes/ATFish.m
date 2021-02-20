//
//  ATFish.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATFish.h"

@implementation ATFish

/// should be obfuscated
- (BOOL)sleepAtNight
{
    return YES;
}

/// should be obfuscated
- (void)swim
{
    /// should be obfuscated
    [self swimEx];
}

/// should be obfuscated
- (void)swimEx
{
    NSLog(@"do nothing");
}

/// should be obfuscated
+ (ATFish *)createInstance
{
    return [[ATFish alloc] init];
}

@end
