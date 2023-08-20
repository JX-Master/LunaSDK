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
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

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
        LUNA_WINDOW_API R<Vector<Path>> open_file_dialog(const c8* title, Span<const FileDialogFilter> filters, const Path& initial_dir, FileDialogFlag flags)
        {
            @autoreleasepool
            {
                NSOpenPanel* open_panel = [NSOpenPanel openPanel];
                [open_panel setCanChooseFiles: YES];
                [open_panel setCanChooseDirectories: NO];
                if(test_flags(flags, FileDialogFlag::multi_select))
                {
                    [open_panel setAllowsMultipleSelection: YES];
                }
                else
                {
                    [open_panel setAllowsMultipleSelection: NO];
                }
                if(!initial_dir.empty())
                {
                    auto encoded_path = initial_dir.encode();
                    NSString* path = [NSString stringWithUTF8String: encoded_path.c_str()];
                    NSURL* url = [NSURL fileURLWithPath: path];
                    [open_panel setDirectoryURL: url];
                }
                NSMutableArray<UTType*>* allowed_types = [NSMutableArray array];
                for(auto& filter : filters)
                {
                    for(auto& extension : filter.extensions)
                    {
                        NSString* extension_str = [NSString stringWithUTF8String: extension];
                        UTType* allowed_type = [UTType typeWithFilenameExtension: extension_str];
                        [allowed_types addObject: allowed_type];
                    }
                }
                [open_panel setAllowedContentTypes: allowed_types];
                if(test_flags(flags, FileDialogFlag::any_file))
                {
                    [open_panel setAllowsOtherFileTypes: YES];
                }
                else
                {
                    [open_panel setAllowsOtherFileTypes: NO];
                }
                if([open_panel runModal] == NSModalResponseOK)
                {
                    Vector<Path> ret;
                    NSArray<NSURL*>* urls = [open_panel URLs];
                    NSUInteger num_urls = [urls count];
                    for(NSUInteger i = 0; i < num_urls; ++i)
                    {
                        NSURL* url = [urls objectAtIndex: i];
                        NSString* path = [url path];
                        ret.push_back(Path([path UTF8String]));
                    }
                    return ret;
                }
                return BasicError::interrupted();
            }
        }
        LUNA_WINDOW_API R<Path> save_file_dialog(const c8* title, Span<const FileDialogFilter> filters, const Path& initial_file_path, FileDialogFlag flags)
        {
            @autoreleasepool
            {
                NSSavePanel* save_panel = [NSSavePanel savePanel];
                [save_panel setCanCreateDirectories: YES];
                if(!initial_file_path.empty())
                {
                    Path path = initial_file_path;
                    Name filename = path.back();
                    path.pop_back();
                    auto encoded_path = path.encode();
                    NSString* filename_str = [NSString stringWithUTF8String: filename.c_str()];
                    NSString* dir = [NSString stringWithUTF8String: encoded_path.c_str()];
                    NSURL* url = [NSURL fileURLWithPath: dir];
                    [save_panel setDirectoryURL: url];
                    [save_panel setNameFieldStringValue: filename_str];
                }
                NSMutableArray<UTType*>* allowed_types = [NSMutableArray array];
                for(auto& filter : filters)
                {
                    for(auto& extension : filter.extensions)
                    {
                        NSString* extension_str = [NSString stringWithUTF8String: extension];
                        UTType* allowed_type = [UTType typeWithFilenameExtension: extension_str];
                        [allowed_types addObject: allowed_type];
                    }
                }
                [save_panel setAllowedContentTypes: allowed_types];
                if(test_flags(flags, FileDialogFlag::any_file))
                {
                    [save_panel setAllowsOtherFileTypes: YES];
                }
                else
                {
                    [save_panel setAllowsOtherFileTypes: NO];
                }
                if([save_panel runModal] == NSModalResponseOK)
                {
                    Path ret;
                    NSURL* url = [save_panel URL];
                    NSString* path = [url path];
                    ret.assign([path UTF8String]);
                    return ret;
                }
                return BasicError::interrupted();
            }
        }
        LUNA_WINDOW_API R<Path> open_dir_dialog(const c8* title, const Path& initial_dir)
        {
            @autoreleasepool
            {
                NSOpenPanel* open_panel = [NSOpenPanel openPanel];
                [open_panel setCanChooseFiles: NO];
                [open_panel setCanChooseDirectories: YES];
                [open_panel setAllowsMultipleSelection: NO];
                if(!initial_dir.empty())
                {
                    auto encoded_path = initial_dir.encode();
                    NSString* path = [NSString stringWithUTF8String: encoded_path.c_str()];
                    NSURL* url = [NSURL fileURLWithPath: path];
                    [open_panel setDirectoryURL: url];
                }
                if([open_panel runModal] == NSModalResponseOK)
                {
                    Path ret;
                    NSURL* url = [[open_panel URLs] objectAtIndex: 0];
                    NSString* path = [url path];
                    ret.assign([path UTF8String]);
                    return ret;
                }
                return BasicError::interrupted();
            }
        }
    }
}