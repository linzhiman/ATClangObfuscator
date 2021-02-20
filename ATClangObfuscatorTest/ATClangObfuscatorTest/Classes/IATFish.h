//
//  IATFish.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol IATFish <NSObject>

/// should be obfuscated
- (void)obfuscate_swim;

@end

NS_ASSUME_NONNULL_END
