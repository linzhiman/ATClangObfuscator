//
//  ATMonster+Action.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import "ATMonster.h"

NS_ASSUME_NONNULL_BEGIN

@interface ATMonster (Action)

@property (nonatomic, assign) BOOL alive;

- (void)doAction;
+ (void)doClassAction;

@end

@protocol IATActionBase <NSObject>

- (NSString *)actionBaseName;

@end

@interface ATMonster (Action2)<IATActionBase>

@end

NS_ASSUME_NONNULL_END
