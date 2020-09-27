//
//  ATFish.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ATAnimal.h"
#import "IATFish.h"

NS_ASSUME_NONNULL_BEGIN

@interface ATFish : ATAnimal<IATFish>

- (void)swimABC;

+ (ATFish *)createInstance;

@end

NS_ASSUME_NONNULL_END
