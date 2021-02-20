//
//  ATAnimal.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/17.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ATAnimal : NSObject

/// all property should NOT be obfuscated

@property (nonatomic, assign) int type;
@property (nonatomic, assign) int age;
@property (nonatomic, strong) NSString *area;
@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) NSString *className;
@property (nonatomic, strong) NSString *subClassName;

/// getter/setter should NOT be obfuscated
- (int)type;

/// getter/setter should NOT be obfuscated
- (void)setType:(int)type;

/// getter/setter should NOT be obfuscated
- (int)age;

/// getter/setter should NOT be obfuscated
- (void)setArea:(NSString *)area;

/// should be obfuscated
- (void)obfuscate_sleep;

/// should be obfuscated
- (void)obfuscate_sleep_now:(BOOL)now duration:(NSInteger)duration;

/// should be obfuscated
- (BOOL)obfuscate_sleep_at_night;

/// should be obfuscated
+ (BOOL)obfuscate_is_any_instance_live;

@end

NS_ASSUME_NONNULL_END
