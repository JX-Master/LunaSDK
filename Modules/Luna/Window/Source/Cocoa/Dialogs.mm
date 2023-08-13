/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Dialogs.mm
* @author JXMaster
* @date 2023/8/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../MessageBox.hpp"
#include "../../FileDialog.hpp"
#include "../../Window.hpp"

#import <Cocoa/Cocoa.h>

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API R<MessageBoxButton> message_box(const c8* text, const c8* caption, MessageBoxType type, MessageBoxIcon icon)
        {
            @autoreleasepool
            {
                NSAlert* alert = [[NSAlert alloc] init];
                NSString* title = [NSString stringWithUTF8String: caption];
                NSString* info = [NSString stringWithUTF8String: text];
                [alert setMessageText: title];
                [alert setInformativeText: info];
                switch(icon)
                {
                    case MessageBoxIcon::none:
                    case MessageBoxIcon::information:
                    case MessageBoxIcon::question:
                        [alert setAlertStyle: NSAlertStyleInformational];
                        break;
                    case MessageBoxIcon::warning:
                        [alert setAlertStyle: NSAlertStyleWarning];
                        break;
                    case MessageBoxIcon::error:
                        [alert setAlertStyle: NSAlertStyleCritical];
                        break;
                }
                switch(type)
                {
                    case MessageBoxType::ok:
                        [alert addButtonWithTitle: @"OK"];
                        break;
                    case MessageBoxType::ok_cancel:
                        [alert addButtonWithTitle: @"OK"];
                        [alert addButtonWithTitle: @"Cancel"];
                        break;
                    case MessageBoxType::retry_cancel:
                        [alert addButtonWithTitle: @"Retry"];
                        [alert addButtonWithTitle: @"Cancel"];
                        break;
                    case MessageBoxType::yes_no:
                        [alert addButtonWithTitle: @"Yes"];
                        [alert addButtonWithTitle: @"No"];
                        break;
                    case MessageBoxType::yes_no_cancel:
                        [alert addButtonWithTitle: @"Yes"];
                        [alert addButtonWithTitle: @"No"];
                        [alert addButtonWithTitle: @"Cancel"];
                        break;
                }
                NSModalResponse response = [alert runModal];
                if(response == NSModalResponseCancel)
                {
                    return MessageBoxButton::cancel;
                }
                if(response == NSModalResponseOK)
                {
                    return MessageBoxButton::ok;
                }
                if(response == NSModalResponseStop || response == NSModalResponseAbort)
                {
                    return BasicError::interrupted();
                }
                switch(type)
                {
                    case MessageBoxType::ok:
                        if(response == NSAlertFirstButtonReturn)
                        {
                            return MessageBoxButton::ok;
                        }
                        break;
                    case MessageBoxType::ok_cancel:
                        if(response == NSAlertFirstButtonReturn)
                        {
                            return MessageBoxButton::ok;
                        }
                        else if(response == NSAlertSecondButtonReturn)
                        {
                            return MessageBoxButton::cancel;
                        }
                        break;
                    case MessageBoxType::retry_cancel:
                        if(response == NSAlertFirstButtonReturn)
                        {
                            return MessageBoxButton::retry;
                        }
                        else if(response == NSAlertSecondButtonReturn)
                        {
                            return MessageBoxButton::cancel;
                        }
                        break;
                    case MessageBoxType::yes_no:
                        if(response == NSAlertFirstButtonReturn)
                        {
                            return MessageBoxButton::yes;
                        }
                        else if(response == NSAlertSecondButtonReturn)
                        {
                            return MessageBoxButton::no;
                        }
                        break;
                    case MessageBoxType::yes_no_cancel:
                        if(response == NSAlertFirstButtonReturn)
                        {
                            return MessageBoxButton::yes;
                        }
                        else if(response == NSAlertSecondButtonReturn)
                        {
                            return MessageBoxButton::no;
                        }
                        else if(response == NSAlertThirdButtonReturn)
                        {
                            return MessageBoxButton::cancel;
                        }
                        break;
                }
                return BasicError::bad_platform_call();
            }
        }
    }
}