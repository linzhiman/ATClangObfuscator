//
//  IATMonster.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol IATMonster <NSObject>

/// should be obfuscated
- (void)scare;

/// should NOT be obfuscated : ATMonster conforms to IATMonster and it has property named amIBig
- (void)amIBig;

/// should NOT be obfuscated : ATMonster conforms to IATMonster and it has property named amISmall
- (void)amISmall;

@end

@protocol IATMonsterEx <IATMonster>

/// should NOT be obfuscated
@property (nonatomic, assign, readonly) BOOL level;

/// should NOT be obfuscated
@property (nonatomic, assign) BOOL hasEye;

/// should be obfuscated
- (void)scare;

/// should NOT be obfuscated : ATMonster conforms to IATMonster and it has property named amIBig
- (void)amIBig;

/// should be obfuscated
- (void)scareEx;

@end

NS_ASSUME_NONNULL_END
