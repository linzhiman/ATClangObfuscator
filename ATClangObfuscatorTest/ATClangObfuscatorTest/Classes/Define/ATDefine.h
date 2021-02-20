//
//  ATDefine.h
//  ATClangObfuscatorTest
//
//  Created by linzhiman on 2021/1/29.
//  Copyright © 2021 AppToolbox. All rights reserved.
//

#ifndef ATDefine_h
#define ATDefine_h

/// createObj should be obfuscated

#define AT_DECLARE_CREATE_OBJ \
+ (instancetype)obfuscate_create_obj;

#define AT_IMPLEMENT_CREATE_OBJ(atType) \
+ (instancetype)obfuscate_create_obj { \
    return [[atType alloc] init]; \
}

#endif /* ATDefine_h */
