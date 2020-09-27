//
//  ATTest.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATTest.h"
#import "ATFish.h"
#import "ATBird.h"
#import "ATMonster.h"
#import "ATMonster+Action.h"

#define ATMasterCenter [ATMonsterCenter sharedObject]

@implementation ATTest

AT_IMPLEMENT_SINGLETON(ATTest);

- (instancetype)init
{
    self = [super init];
    if (self) {
        AT_ADD_NOTIFY_DEFAULT_SELECTOR_NAME(ATMonsterCreate);
    }
    return self;
}

- (void)test
{
    ATFish *fish = [ATFish createInstance];
    [fish swim];
    [fish swimABC];
    
    id<IATBird> iBird = [ATBird new];
    [iBird fly];
    
    ATBird *bird = [ATBird new];
    BOOL full = bird.full;
    bird.full = full;
}

- (void)setMonster:(ATMonster *)monster
{
    [monster sleep];
    [monster doAction];
    [monster performSelector:@selector(selectorShouldNotObfuscator)];
    
    [ATMonster doClassAction];
}

AT_HANDLER_NOTIFY_DEFAULT_SELECTOR_NAME(ATMonsterCreate)
{
    [ATMasterCenter create];
    ATMasterCenterCreate;
}

@end
