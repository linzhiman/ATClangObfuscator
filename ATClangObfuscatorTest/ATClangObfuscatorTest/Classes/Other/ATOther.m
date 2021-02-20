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
        [self obfuscate_other];
    }
    return self;
}

/// should be obfuscated
- (void)obfuscate_other
{
    /// sys method should NOT be obfuscated
    UIView *tmp = [[UIView alloc] initWithFrame:CGRectZero];
    
    /// sys method should NOT be obfuscated
    [tmp sizeToFit];
    
    /// should be obfuscated
    [self obfuscate_private];
    
    /// sys method should NOT be obfuscated
    ATAnimal *animal = [ATAnimal new];
    
    /// should be obfuscated
    [animal obfuscate_sleep];
}

/// should be obfuscated
- (void)obfuscate_private
{
    ;;
}

@end


@implementation NSObject (ATOther)

/// should be obfuscated
- (void)obfuscate_public_method
{
    /// should be obfuscated
    [self obfuscate_private_method];
}

/// should be obfuscated
- (void)obfuscate_private_method
{
    ;;
}

@end
