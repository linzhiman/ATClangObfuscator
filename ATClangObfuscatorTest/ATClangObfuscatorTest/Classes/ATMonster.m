//
//  ATMonster.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATMonster.h"

AT_DECLARE_NOTIFI(ATMonsterCreate);

@interface ATMonsterView()<UITableViewDataSource, UITableViewDelegate>

@end

@implementation ATMonsterView

/// sys method should NOT be obfuscated
- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        UIView *sub = [UIView new];
        [self addSubview:sub];
        [sub removeFromSuperview];
    }
    return self;
}

/// sys method should NOT be obfuscated
- (void)layoutSubviews
{
    [super layoutSubviews];
}

/// sys method should NOT be obfuscated
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 0;
}

/// sys method should NOT be obfuscated
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 0;
}

/// sys method should NOT be obfuscated
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return nil;
}

/// sys method should NOT be obfuscated
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    /// sys method should NOT be obfuscated
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

/// sys method should NOT be obfuscated
- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 0;
}

/// sys method should NOT be obfuscated
- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    return nil;
}

@end

@interface ATMonster ()<IATMonster>

/// property should NOT be obfuscated
@property (nonatomic, assign) BOOL amISmall;

@end

@implementation ATMonster

/// should be obfuscated
- (void)obfuscate_swim
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)obfuscate_fly
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)obfuscate_scare
{
    /// should be obfuscated
    [self obfuscate_private_scare];
}

/// should be obfuscated
- (void)obfuscate_private_scare
{
    /// should be obfuscated
    [self obfuscate_private_scare_ex];
}

/// should be obfuscated
- (void)obfuscate_private_scare_ex
{
    NSLog(@"do nothing");
}

/// should NOT be obfuscated : ATTest call this with performSelector
- (void)run
{
    /// should be obfuscated
    [self obfuscate_private_scare_ex];
}

@end


@implementation ATMonsterEx

/// should be obfuscated
- (void)obfuscate_scare
{
    NSLog(@"do nothing");
}

/// should NOT be obfuscated : ATMonster conforms to IATMonster and it has property named amIBig
- (void)amIBig
{
    NSLog(@"do nothing");
}

/// should NOT be obfuscated : ATMonster conforms to IATMonster and it has property named amISmall
- (void)amISmall
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)obfuscate_scare_ex
{
    NSLog(@"do nothing");
}

@end


@implementation ATMonsterEx (OnlyImp)

/// should be obfuscated
- (void)obfuscate_only_imp
{
    NSLog(@"do nothing");
}

@end


@interface ATMonsterCenter ()

/// property should NOT be obfuscated
@property (nonatomic, weak) ATMonsterView *currentMonsterView;

@end

@implementation ATMonsterCenter

AT_IMPLEMENT_SINGLETON(ATMonsterCenter);

/// should be obfuscated
- (ATMonster *)obfuscate_create
{
    AT_POST_NOTIFY(ATMonsterCreate);
    return [ATMonster new];
}

/// should be obfuscated
- (ATMonsterEx *)obfuscate_create_ex
{
    /// new should NOT be obfuscated
    ATMonsterEx *ex = [ATMonsterEx new];
    
    /// should be obfuscated
    [ex obfuscate_only_imp];
    
    return ex;
}

@end
