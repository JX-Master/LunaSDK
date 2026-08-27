/*!
* This file is a portion of LunaSDK.
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
#include <Luna/Runtime/TSAssert.hpp>

#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API R<usize> message_box(const c8* text, const c8* title, Span<const c8*> buttons,
            MessageBoxIcon icon, usize default_button_index, usize cancel_button_index)
        {
            lutsassert_main_thread();
            if(!text || !title || buttons.empty() || default_button_index >= buttons.size() ||
                (cancel_button_index != USIZE_MAX && (cancel_button_index >= buttons.size() ||
                    cancel_button_index == default_button_index)))
            {
                return E_BAD_ARGUMENTS;
            }
            @autoreleasepool
            {
                NSString* message_text = [NSString stringWithUTF8String: text];
                NSString* message_title = [NSString stringWithUTF8String: title];
                if(!message_text || !message_title) return E_BAD_ARGUMENTS;

                NSAlert* alert = [[NSAlert alloc] init];
                if(!alert) return E_OUT_OF_MEMORY;
                [alert setMessageText: message_title];
                [alert setInformativeText: message_text];
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
                    default:
                        return E_BAD_ARGUMENTS;
                }

                for(const c8* button_text : buttons)
                {
                    if(!button_text || !button_text[0]) return E_BAD_ARGUMENTS;
                    NSString* native_button_text = [NSString stringWithUTF8String: button_text];
                    if(!native_button_text) return E_BAD_ARGUMENTS;
                    if(![alert addButtonWithTitle: native_button_text]) return E_OUT_OF_MEMORY;
                }

                NSArray<NSButton*>* native_buttons = [alert buttons];
                for(NSButton* button in native_buttons)
                {
                    [button setKeyEquivalent: @""];
                    [button setKeyEquivalentModifierMask: 0];
                }
                NSButton* default_button = [native_buttons objectAtIndex: default_button_index];
                [default_button setKeyEquivalent: @"\r"];
                if(cancel_button_index != USIZE_MAX)
                {
                    NSButton* cancel_button = [native_buttons objectAtIndex: cancel_button_index];
                    [cancel_button setKeyEquivalent: @"\033"];
                }

                NSModalResponse response = [alert runModal];
                if(response >= NSAlertFirstButtonReturn)
                {
                    usize button_index = (usize)(response - NSAlertFirstButtonReturn);
                    if(button_index < buttons.size()) return button_index;
                }
                if(response == NSModalResponseCancel || response == NSModalResponseStop ||
                    response == NSModalResponseAbort)
                {
                    return E_INTERRUPTED;
                }
                return set_error(E_BAD_PLATFORM_CALL, "NSAlert returned unexpected modal response: %lld.",
                    (i64)response);
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
                return E_INTERRUPTED;
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
                return E_INTERRUPTED;
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
                return E_INTERRUPTED;
            }
        }
    }
}
