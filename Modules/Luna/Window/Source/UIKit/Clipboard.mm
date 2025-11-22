/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Clipboard.mm
* @author JXMaster
* @date 2025/11/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../Clipboard.hpp"

#import <UIKit/UIKit.h>

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API RV get_clipboard_text(String& out_text)
        {
            @autoreleasepool
            {
                UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
                NSString *string = pasteboard.string;
                
                if (!string)
                {
                    // Got no text from clipboard, just return normally.
                    return ok;
                }
                
                const char* utf8String = [string UTF8String];
                if (utf8String)
                {
                    out_text.append(utf8String);
                }
                
                return ok;
            }
        }
        
        LUNA_WINDOW_API RV set_clipboard_text(const c8* text, usize size)
        {
            @autoreleasepool 
            {
                if (text && *text) 
                {
                    size = min<usize>(size, strlen(text));
                    NSString* string = [[NSString alloc] initWithBytes:text length:size encoding:NSUTF8StringEncoding];
                    [UIPasteboard generalPasteboard].string = string;
                }
                else 
                {
                    [UIPasteboard generalPasteboard].string = nil;
                }
                return ok;
            }
        }
    }
}

