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

- (void)layoutSubviews
{
    [super layoutSubviews];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 0;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return nil;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 0;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    return nil;
}

@end

@interface ATMonster ()<IATMonster>

@property (nonatomic, assign) BOOL amISmall;

@end

@implementation ATMonster

- (void)swim
{
    ;;
}

- (void)fly
{
    ;;
}

- (void)scare
{
    [self innerScare];
}

- (void)innerScare
{
    [self innerScare2];
}

- (void)innerScare2
{
    ;;
}

- (void)selectorShouldNotObfuscator
{
    [self innerScare2];
}

@end


@implementation ATMonsterCenter

AT_IMPLEMENT_SINGLETON(ATMonsterCenter);

- (ATMonster *)create
{
    AT_POST_NOTIFY(ATMonsterCreate);
    return [ATMonster new];
}

@end
