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

#define ATMasterCenter [ATMonsterCenter sharedObject]
#define ATMasterCenterCreate [ATMasterCenter create];

@class ATMonster;

@interface ATMonsterView : UIView

@end

@interface ATMonster : ATAnimal<IATFish, IATBird>

@property (nonatomic, assign) BOOL amIBig;

- (void)selectorShouldNotObfuscator;

@end

@interface ATMonsterCenter : UIView

AT_DECLARE_SINGLETON;

- (ATMonster *)create;

@end

NS_ASSUME_NONNULL_END
