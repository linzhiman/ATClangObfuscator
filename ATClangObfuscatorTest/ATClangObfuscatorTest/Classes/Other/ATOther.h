//
//  ATOther.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2021/2/7.
//  Copyright © 2021 AppToolbox. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface ATOther : NSObject

@end

@interface NSObject (ATOther)

/// should be obfuscated
- (void)publicMethod;

@end

NS_ASSUME_NONNULL_END
