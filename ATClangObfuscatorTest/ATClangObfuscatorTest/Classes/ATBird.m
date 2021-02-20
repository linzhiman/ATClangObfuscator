//
//  ATBird.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATBird.h"

@implementation ATBird

/// should be obfuscated
- (BOOL)obfuscate_sleep_at_night
{
    return YES;
}

/// should be obfuscated
- (void)obfuscate_fly
{
    /// should be obfuscated
    [self obfuscate_fly_ex];
}

/// should be obfuscated
- (void)obfuscate_fly_ex
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (int)obfuscate_count
{
    return 0;
}

/// should be obfuscated
- (void)setObfuscate_count:(int)count
{
    NSLog(@"do nothing");
}

@end
