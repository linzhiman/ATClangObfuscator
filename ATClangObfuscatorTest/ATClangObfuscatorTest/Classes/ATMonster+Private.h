//
//  ATMonster+Private.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2021/2/23.
//  Copyright © 2021 AppToolbox. All rights reserved.
//

#import "ATMonster.h"

NS_ASSUME_NONNULL_BEGIN

@interface ATMonster ()

/// property should NOT be obfuscated
@property (nonatomic, strong) NSString *name;

@end

NS_ASSUME_NONNULL_END
