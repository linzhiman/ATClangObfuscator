//
//  ATMonster.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ATAnimal.h"
#import "IATFish.h"
#import "IATBird.h"
#import "ATMacro.h"

NS_ASSUME_NONNULL_BEGIN

AT_EXTERN_NOTIFI(ATMonsterCreate);

/// should be obfuscated
#define ATMonsterCenterObj [ATMonsterCenter sharedObject]

/// should be obfuscated
#define ATMonsterCenterObjCreate [ATMonsterCenterObj create];

@class ATMonster;

@interface ATMonsterView : UIView

@end

@interface ATMonster : ATAnimal<IATFish, IATBird>

/// property should NOT be obfuscated
@property (nonatomic, assign) BOOL amIBig;

/// should NOT be obfuscated : ATTest call this with performSelector
- (void)run;

@end

@interface ATMonsterCenter : UIView

AT_DECLARE_SINGLETON;

/// should be obfuscated
- (ATMonster *)create;

@end

NS_ASSUME_NONNULL_END
