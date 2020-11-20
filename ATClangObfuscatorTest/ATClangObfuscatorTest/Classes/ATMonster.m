//
//  ATMonster.m
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATMonster.h"
#import "IATMonster.h"

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
- (void)swim
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)fly
{
    NSLog(@"do nothing");
}

/// should be obfuscated
- (void)scare
{
    /// should be obfuscated
    [self privateScare];
}

/// should be obfuscated
- (void)privateScare
{
    /// should be obfuscated
    [self privateScareEx];
}

/// should be obfuscated
- (void)privateScareEx
{
    NSLog(@"do nothing");
}

/// should NOT be obfuscated : ATTest call this with performSelector
- (void)run
{
    /// should be obfuscated
    [self privateScareEx];
}

@end


@implementation ATMonsterCenter

AT_IMPLEMENT_SINGLETON(ATMonsterCenter);

/// should be obfuscated
- (ATMonster *)create
{
    AT_POST_NOTIFY(ATMonsterCreate);
    return [ATMonster new];
}

@end
