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
- (BOOL)obfuscate_sleep_at_night
{
    return YES;
}

/// should be obfuscated
- (void)obfuscate_swim
{
    /// should be obfuscated
    [self obfuscate_swim_ex];
}

/// should be obfuscated
- (void)obfuscate_swim_ex
{
    NSLog(@"do nothing");
}

/// should be obfuscated
+ (ATFish *)obfuscate_create_instance
{
    return [[ATFish alloc] init];
}

@end
