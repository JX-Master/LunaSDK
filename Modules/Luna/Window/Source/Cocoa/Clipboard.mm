/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Clipboard.mm
* @author JXMaster
* @date 2025/10/9
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../Clipboard.hpp"

#import <Cocoa/Cocoa.h>

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API RV get_clipboard_text(String& out_text)
        {
            @autoreleasepool
            {
                NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
                NSString* text = [pasteboard stringForType:NSPasteboardTypeString];
                
                if (!text)
                {
                    // Got no text from clipboard, just return normally.
                    return ok;
                }
                
                const char* utf8String = [text UTF8String];
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
                lucheck(text);
                
                // Calculate actual size
                usize actual_size = size;
                // Find null terminator within size limit
                for (usize i = 0; i < size; ++i)
                {
                    if (text[i] == '\0')
                    {
                        actual_size = i;
                        break;
                    }
                }
                
                NSString* nsString = [[NSString alloc] initWithBytes:text
                                                              length:actual_size
                                                            encoding:NSUTF8StringEncoding];
                if (!nsString)
                {
                    return BasicError::out_of_memory();
                }
                
                NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
                [pasteboard clearContents];
                BOOL success = [pasteboard setString:nsString forType:NSPasteboardTypeString];
                
                if (!success)
                {
                    return BasicError::bad_platform_call();
                }
                
                return ok;
            }
        }
    }
}

