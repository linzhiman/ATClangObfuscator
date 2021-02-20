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

@implementation ATTest

AT_IMPLEMENT_SINGLETON(ATTest);
AT_IMPLEMENT_CREATE_OBJ(ATTest);

/// sys method should NOT be obfuscated
- (instancetype)init
{
    self = [super init];
    if (self) {
        AT_ADD_NOTIFY_DEFAULT_SELECTOR_NAME(ATMonsterCreate);
    }
    return self;
}

/// should be obfuscated
- (void)obfuscate_test
{
    /// should be obfuscated
    ATFish *fish = [ATFish obfuscate_create_instance];
    
    /// should be obfuscated
    [fish obfuscate_swim];
    
    /// should be obfuscated
    [fish obfuscate_swim_ex];
    
    /// new should NOT be obfuscated
    id<IATBird> iBird = [ATBird new];
    
    /// should be obfuscated
    [iBird obfuscate_fly];
    
    /// new should NOT be obfuscated
    ATBird *bird = [ATBird new];
    
    /// property should NOT be obfuscated
    int age = [bird age];
    
    /// property should NOT be obfuscated
    [bird setAge:age];
    
    /// property should NOT be obfuscated
    BOOL full = bird.full;
    
    /// property should NOT be obfuscated
    bird.full = full;
    
    /// not property should be obfuscated
    int count = bird.obfuscate_count;
    
    /// not property should be obfuscated
    bird.obfuscate_count = count;
}

/// should be obfuscated
- (void)setObfuscate_monster:(ATMonster *)monster
{
    /// should be obfuscated
    [monster obfuscate_sleep];
    
    /// should be obfuscated
    [monster obfuscate_do_action];
    
    /// run should NOT be obfuscated
    [monster performSelector:@selector(run)];
    
    /// should be obfuscated
    [ATMonster obfuscate_do_class_action];
    
    id tmp = monster;
    
    /// should be obfuscated
    [tmp obfuscate_sleep];
    
    /// run should NOT be obfuscated
    [tmp run];
    
    /// new should NOT be obfuscated
    id<IATMonster> monsterEx = [ATMonsterEx new];
    
    /// should be obfuscated
    [monsterEx obfuscate_scare];
    
    /// should be obfuscated
    ATTest *newObj = [ATTest obfuscate_create_obj];
    
    /// should be obfuscated
    [newObj obfuscate_test];
    
    /// onATMonsterCreate is written in ScratchSpace should NOT be obfuscated
    [self onATMonsterCreate:nil];
}

AT_HANDLER_NOTIFY_DEFAULT_SELECTOR_NAME(ATMonsterCreate)
{
    /// should be obfuscated
    [ATMonsterCenterObj obfuscate_create];
    
    ATMonsterCenterObjCreate;
    
    /// getter/setter should NOT be obfuscated
    UIView *tmp = [ATMonsterCenterObj currentMonsterView];
    
    /// getter/setter should NOT be obfuscated
    if (tmp == ATMonsterCenterObj.currentMonsterView) {
        tmp = nil;
    }
}

@end
