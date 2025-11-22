/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file UIKitAppDelegate.mm
* @author JXMaster
* @date 2025/11/19
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../../UIKit/AppMainUIKit.hpp"
#include "../../Event.hpp"

#import "UIKitAppDelegate.h"
#import "Window.h"

namespace Luna
{
    namespace Window
    {
        static int (*g_luna_main_func)(int argc, const char* argv[]);
        static int g_argc;
        static char** g_argv;
        static int g_exit_status = 0;

        bool g_pump_events = false;

        LUNA_WINDOW_API int uikit_app_run(int argc, char *argv[], int (*luna_main_func)(int argc, const char* argv[]))
        {
            g_luna_main_func = luna_main_func;
            g_argc = argc;
            g_argv = argv;
            @autoreleasepool
            {
                UIApplicationMain(argc, argv, nil, [LunaUIKitDelegate getAppDelegateClassName]);
            }
            return g_exit_status;
        }
    }
}

using namespace Luna;
using namespace Luna::Window;

@interface LunaLifecycleObserver : NSObject
{
    BOOL _isObserving;
}
@property(assign, nonatomic) BOOL isObserving;
@end

@implementation LunaLifecycleObserver
- (instancetype) init
{
    self = [super init];
    _isObserving = NO;
    return self;
}

@dynamic isObserving;
- (BOOL)isIsObserving
{
    return _isObserving;
}
- (void)setIsObserving:(BOOL)isObserving 
{
    NSNotificationCenter *notificationCenter = NSNotificationCenter.defaultCenter;
    if(isObserving && !_isObserving)
    {
        //[notificationCenter addObserver:self selector:@selector(applicationDidBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
        //[notificationCenter addObserver:self selector:@selector(applicationWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];
        //[notificationCenter addObserver:self selector:@selector(applicationDidEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
        //[notificationCenter addObserver:self selector:@selector(applicationWillEnterForeground) name:UIApplicationWillEnterForegroundNotification object:nil];
        [notificationCenter addObserver:self selector:@selector(applicationWillTerminate) name:UIApplicationWillTerminateNotification object:nil];
        [notificationCenter addObserver:self selector:@selector(applicationDidReceiveMemoryWarning) name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
    }
    if(!isObserving && _isObserving)
    {
        [notificationCenter removeObserver:self];
    }
    _isObserving = isObserving;
}

// - (void)applicationDidBecomeActive
// {
//     auto e = new_object<ApplicationDidEnterForegroundEvent>();
//     dispatch_event_to_handler(e.object());
// }

// - (void)applicationWillResignActive
// {
//     auto e = new_object<ApplicationWillEnterBackgroundEvent>();
//     dispatch_event_to_handler(e.object());
// }

// - (void)applicationDidEnterBackground
// {
//     auto e = new_object<ApplicationDidEnterBackgroundEvent>();
//     dispatch_event_to_handler(e.object());
// }

// - (void)applicationWillEnterForeground
// {
//     auto e = new_object<ApplicationWillEnterForegroundEvent>();
//     dispatch_event_to_handler(e.object());
// }

- (void)applicationWillTerminate
{
    auto e = new_object<ApplicationWillTerminateEvent>();
    dispatch_event_to_handler(e.object());
}

- (void)applicationDidReceiveMemoryWarning
{
    auto e = new_object<ApplicationDidReceiveMemoryWarningEvent>();
    dispatch_event_to_handler(e.object());
}

@end

@implementation LunaUIKitDelegate

+ (id)sharedAppDelegate
{
    return [UIApplication sharedApplication].delegate;
}

+ (NSString*)getAppDelegateClassName
{
    return @"LunaUIKitDelegate";
}

@synthesize primary_scene;

- (instancetype)init
{
    self = [super init];
    primary_scene = nil;
    return self;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    return YES;
}

- (UISceneConfiguration*) application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options 
{
    UISceneConfiguration* config = [[UISceneConfiguration alloc] initWithName:@"LunaSceneConfiguration" sessionRole:connectingSceneSession.role];
    config.delegateClass = [LunaUIKitSceneDelegate class];
    return config;
}

@end

@implementation LunaUIKitSceneDelegate

- (void)runMainFunction
{
    LunaLifecycleObserver *lifecycleObserver = [[LunaLifecycleObserver alloc] init];
    g_pump_events = YES;
    lifecycleObserver.isObserving = YES;
    g_exit_status = g_luna_main_func(g_argc, (const char**)g_argv);
    lifecycleObserver.isObserving = NO;
    g_pump_events = NO;
}

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions
{
    if (![scene isKindOfClass:[UIWindowScene class]]) {
        return;
    }

    UIWindowScene *windowScene = (UIWindowScene *)scene;
    windowScene.delegate = self;

    id delegate = [UIApplication sharedApplication].delegate;
    if([delegate isKindOfClass:[LunaUIKitDelegate class]])
    {
        LunaUIKitDelegate* luna_delegate = (LunaUIKitDelegate*)delegate;
        if(luna_delegate.primary_scene == nil)
        {
            // Main scene connected.
            luna_delegate.primary_scene = windowScene;
            [self performSelector:@selector(runMainFunction) withObject: nil afterDelay: 0.0];
        }
    }
}

- (void)sceneDidBecomeActive:(UIScene *)scene
{
    if(g_window)
    {
        g_window->m_minimized = false;
    }
    auto e = new_object<ApplicationDidEnterForegroundEvent>();
    dispatch_event_to_handler(e.object());
}

- (void)sceneWillResignActive:(UIScene *)scene
{
    auto e = new_object<ApplicationWillEnterBackgroundEvent>();
    dispatch_event_to_handler(e.object());
}

- (void)sceneWillEnterForeground:(UIScene *)scene
{
    auto e = new_object<ApplicationWillEnterForegroundEvent>();
    dispatch_event_to_handler(e.object());
}

- (void)sceneDidEnterBackground:(UIScene *)scene
{
    if(g_window)
    {
        g_window->m_minimized = true;
    }
    auto e = new_object<ApplicationDidEnterBackgroundEvent>();
    dispatch_event_to_handler(e.object());
}

@end