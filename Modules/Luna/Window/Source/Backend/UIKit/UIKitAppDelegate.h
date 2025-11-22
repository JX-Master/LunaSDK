/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file UIKitAppDelegate.hpp
* @author JXMaster
* @date 2025/11/19
*/
#pragma once

#import <UIKit/UIKit.h>

namespace Luna
{
    namespace Window
    {
        extern bool g_pump_events;
    }
}

@interface LunaUIKitDelegate : NSObject <UIApplicationDelegate>

@property(readwrite, nonatomic) UIWindowScene* primary_scene;

- (instancetype)init;

+ (id)sharedAppDelegate;
+ (NSString *)getAppDelegateClassName;

@end

@interface LunaUIKitSceneDelegate : NSObject <UISceneDelegate>

@end