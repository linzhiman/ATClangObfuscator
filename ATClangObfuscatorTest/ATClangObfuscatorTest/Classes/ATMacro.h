//
//  ATMacro.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2020/9/18.
//  Copyright © 2020 AppToolbox. All rights reserved.
//

#ifndef ATMacro_h
#define ATMacro_h

//Singleton

#define AT_DECLARE_SINGLETON \
+ (instancetype)sharedObject;

#define AT_IMPLEMENT_SINGLETON(atType) \
+ (instancetype)sharedObject { \
    static dispatch_once_t __once; \
    static atType *__instance = nil; \
    dispatch_once(&__once, ^{ \
        __instance = [[atType alloc] init]; \
    }); \
    return __instance; \
}

//Notification
#define AT_DECLARE_NOTIFI(__NAME__)  NSString* const __NAME__ = @#__NAME__;
#define AT_EXTERN_NOTIFI(__NAME__)   extern NSString *const __NAME__;

#define AT_ADD_NOTIFY(__NAME__, __SELECTOR__) [[NSNotificationCenter defaultCenter] addObserver:self selector:__SELECTOR__ name:__NAME__ object:nil];
#define AT_ADD_NOTIFY_DEFAULT_SELECTOR_NAME(__NAME__) AT_ADD_NOTIFY(__NAME__, @selector(on##__NAME__:))
#define AT_HANDLER_NOTIFY_DEFAULT_SELECTOR_NAME(__NAME__) - (void)on##__NAME__:(NSNotification *)notification

#define AT_POST_NOTIFY(__NAME__)   [[NSNotificationCenter defaultCenter] postNotificationName:__NAME__ object:nil]

#endif /* ATMacro_h */
