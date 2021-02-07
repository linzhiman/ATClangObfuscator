//
//  ATOther.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2021/2/7.
//  Copyright © 2021 AppToolbox. All rights reserved.
//

#import "ATOther.h"
#import "ATAnimal.h"

@implementation ATOther

/// sys method should NOT be obfuscated
- (instancetype)init
{
    self = [super init];
    if (self) {
        /// should be obfuscated
        [self other];
    }
    return self;
}

/// should be obfuscated
- (void)other
{
    /// sys method should NOT be obfuscated
    UIView *tmp = [[UIView alloc] initWithFrame:CGRectZero];
    
    /// sys method should NOT be obfuscated
    [tmp sizeToFit];
    
    /// should be obfuscated
    [self private];
    
    /// sys method should NOT be obfuscated
    ATAnimal *animal = [ATAnimal new];
    
    /// should be obfuscated
    [animal sleep];
}

/// should be obfuscated
- (void)private
{
    ;;
}

@end


@implementation NSObject (ATOther)

/// should be obfuscated
- (void)publicMethod
{
    /// should be obfuscated
    [self privateMethod];
}

/// should be obfuscated
- (void)privateMethod
{
    ;;
}

@end
