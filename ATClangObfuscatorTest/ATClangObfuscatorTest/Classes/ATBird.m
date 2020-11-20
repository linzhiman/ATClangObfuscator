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
- (BOOL)sleepAtNight
{
    return YES;
}

/// should be obfuscated
- (void)fly
{
    /// should be obfuscated
    [self flyEx];
}

/// should be obfuscated
- (void)flyEx
{
    NSLog(@"do nothing");
}

@end
