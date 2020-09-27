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

@property (nonatomic, strong) NSString *name;
@property (nonatomic, assign) int type;
@property (nonatomic, assign) int age;
@property (nonatomic, strong) NSString *area;
@property (nonatomic, strong) NSString *className;
@property (nonatomic, strong) NSString *subClassName;

- (void)sleep;

- (void)sleepNow:(BOOL)now duration:(NSInteger)duration;

- (BOOL)sleepAtNight;

- (int)type;
- (void)setType:(int)type;

- (int)age;

- (void)setArea:(NSString *)area;

+ (BOOL)isAnyInstanceLive;

@end

NS_ASSUME_NONNULL_END
