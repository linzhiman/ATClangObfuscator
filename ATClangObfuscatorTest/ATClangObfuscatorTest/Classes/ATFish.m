//
//  ATFish.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATFish.h"

@implementation ATFish

- (BOOL)sleepAtNight
{
    return YES;
}

- (void)swim
{
    [self swimABC];
}

- (void)swimABC
{
    ;;
}

+ (ATFish *)createInstance
{
    return [[ATFish alloc] init];
}

@end
