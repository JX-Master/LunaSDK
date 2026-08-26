/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ApplicationMenu.mm
* @author JXMaster
* @date 2026/8/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../ApplicationMenu.hpp"
#include "../Event.hpp"
#include "ApplicationMenu.h"
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

@interface LunaApplicationDelegate : NSObject<NSApplicationDelegate>
@end

@interface LunaApplicationMenuActionTarget : NSObject<NSMenuItemValidation>
- (void)invokeApplicationMenuItem:(NSMenuItem*)sender;
- (void)requestApplicationQuit:(NSMenuItem*)sender;
@end

namespace Luna
{
    namespace Window
    {
        namespace
        {
            constexpr usize MAX_MENU_DEPTH = 64;
            constexpr usize NUM_COCOA_KEY_CODES = 65536;

            __strong LunaApplicationDelegate* g_application_delegate = nil;
            __strong LunaApplicationMenuActionTarget* g_application_menu_action_target = nil;
            __strong NSMenu* g_owned_main_menu = nil;
            __strong NSMenu* g_owned_services_menu = nil;
            __strong NSMenu* g_owned_windows_menu = nil;
            __strong NSMenu* g_owned_help_menu = nil;
            __strong NSMutableDictionary<NSNumber*, NSMenuItem*>* g_items_by_id = nil;
            __strong NSString* g_application_name = nil;
            bool g_application_delegate_attached = false;
            bool g_runtime_bridge_enabled = false;
            bool g_using_default_menu = false;
            bool g_suppressed_key_ups[NUM_COCOA_KEY_CODES] = {};
            u8 g_item_enabled_state_key;

            struct BuiltMenu
            {
                __strong NSMenu* main_menu = nil;
                __strong NSMenu* services_menu = nil;
                __strong NSMenu* windows_menu = nil;
                __strong NSMenu* help_menu = nil;
                __strong NSMutableDictionary<NSNumber*, NSMenuItem*>* items_by_id = nil;
            };

            struct ValidationContext
            {
                HashSet<application_menu_item_id_t> ids;
                bool has_services_menu = false;
                bool has_windows_menu = false;
                bool has_help_menu = false;
            };

            void assert_cocoa_main_thread()
            {
                luassert_msg_always([NSThread isMainThread], "This function must only be called from the main thread.");
            }

            NSString* string_from_utf8(const c8* string)
            {
                if(!string) return nil;
                return [[NSString alloc] initWithUTF8String:string];
            }

            NSString* fallback_application_name()
            {
                NSBundle* bundle = [NSBundle mainBundle];
                id value = [bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"];
                if([value isKindOfClass:[NSString class]] && [(NSString*)value length])
                {
                    return (NSString*)value;
                }
                value = [bundle objectForInfoDictionaryKey:@"CFBundleName"];
                if([value isKindOfClass:[NSString class]] && [(NSString*)value length])
                {
                    return (NSString*)value;
                }
                NSString* process_name = [[NSProcessInfo processInfo] processName];
                return [process_name length] ? process_name : @"Application";
            }

            NSString* current_application_name()
            {
                if(!g_application_name || ![g_application_name length])
                {
                    g_application_name = [fallback_application_name() copy];
                }
                return g_application_name;
            }

            NSString* default_title_for_role(ApplicationMenuItemRole role)
            {
                NSString* app_name = current_application_name();
                switch(role)
                {
                    case ApplicationMenuItemRole::about:
                        return [NSString stringWithFormat:@"About %@", app_name];
                    case ApplicationMenuItemRole::settings:
                        return @"Settings...";
                    case ApplicationMenuItemRole::services:
                        return @"Services";
                    case ApplicationMenuItemRole::hide:
                        return [NSString stringWithFormat:@"Hide %@", app_name];
                    case ApplicationMenuItemRole::hide_others:
                        return @"Hide Others";
                    case ApplicationMenuItemRole::show_all:
                        return @"Show All";
                    case ApplicationMenuItemRole::quit:
                        return [NSString stringWithFormat:@"Quit %@", app_name];
                    case ApplicationMenuItemRole::window_menu:
                        return @"Window";
                    case ApplicationMenuItemRole::help_menu:
                        return @"Help";
                    default:
                        return nil;
                }
            }

            NSString* title_for_item(const ApplicationMenuItemDesc& desc)
            {
                if(desc.title) return string_from_utf8(desc.title);
                return default_title_for_role(desc.role);
            }

            NSString* key_equivalent_for_key(KeyCode key, NSEventModifierFlags& additional_modifiers)
            {
                additional_modifiers = 0;
                if(key >= KeyCode::f1 && key <= KeyCode::f12)
                {
                    unichar character = (unichar)(NSF1FunctionKey + ((u16)key - (u16)KeyCode::f1));
                    return [NSString stringWithCharacters:&character length:1];
                }
                if(key >= KeyCode::num0 && key <= KeyCode::num9)
                {
                    unichar character = (unichar)('0' + ((u16)key - (u16)KeyCode::num0));
                    return [NSString stringWithCharacters:&character length:1];
                }
                if(key >= KeyCode::a && key <= KeyCode::z)
                {
                    unichar character = (unichar)('a' + ((u16)key - (u16)KeyCode::a));
                    return [NSString stringWithCharacters:&character length:1];
                }
                if(key >= KeyCode::numpad0 && key <= KeyCode::numpad9)
                {
                    additional_modifiers = NSEventModifierFlagNumericPad;
                    unichar character = (unichar)('0' + ((u16)key - (u16)KeyCode::numpad0));
                    return [NSString stringWithCharacters:&character length:1];
                }

                unichar character = 0;
                switch(key)
                {
                    case KeyCode::esc: character = 0x1b; break;
                    case KeyCode::grave: character = '`'; break;
                    case KeyCode::equal: character = '='; break;
                    case KeyCode::minus: character = '-'; break;
                    case KeyCode::backspace: character = NSBackspaceCharacter; break;
                    case KeyCode::tab: character = NSTabCharacter; break;
                    case KeyCode::enter: character = NSCarriageReturnCharacter; break;
                    case KeyCode::spacebar: character = ' '; break;
                    case KeyCode::l_branket: character = '['; break;
                    case KeyCode::r_branket: character = ']'; break;
                    case KeyCode::backslash: character = '\\'; break;
                    case KeyCode::semicolon: character = ';'; break;
                    case KeyCode::quote: character = '\''; break;
                    case KeyCode::comma: character = ','; break;
                    case KeyCode::period: character = '.'; break;
                    case KeyCode::slash: character = '/'; break;
                    case KeyCode::print_screen: character = NSPrintScreenFunctionKey; break;
                    case KeyCode::scroll_lock: character = NSScrollLockFunctionKey; break;
                    case KeyCode::pause: character = NSPauseFunctionKey; break;
                    case KeyCode::insert: character = NSInsertFunctionKey; break;
                    case KeyCode::home: character = NSHomeFunctionKey; break;
                    case KeyCode::page_up: character = NSPageUpFunctionKey; break;
                    case KeyCode::page_down: character = NSPageDownFunctionKey; break;
                    case KeyCode::del: character = NSDeleteFunctionKey; break;
                    case KeyCode::end: character = NSEndFunctionKey; break;
                    case KeyCode::left: character = NSLeftArrowFunctionKey; break;
                    case KeyCode::up: character = NSUpArrowFunctionKey; break;
                    case KeyCode::right: character = NSRightArrowFunctionKey; break;
                    case KeyCode::down: character = NSDownArrowFunctionKey; break;
                    case KeyCode::num_lock:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = NSClearLineFunctionKey;
                        break;
                    case KeyCode::numpad_decimal:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = '.';
                        break;
                    case KeyCode::numpad_add:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = '+';
                        break;
                    case KeyCode::numpad_subtract:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = '-';
                        break;
                    case KeyCode::numpad_multiply:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = '*';
                        break;
                    case KeyCode::numpad_divide:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = '/';
                        break;
                    case KeyCode::numpad_equal:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = '=';
                        break;
                    case KeyCode::numpad_enter:
                        additional_modifiers = NSEventModifierFlagNumericPad;
                        character = NSEnterCharacter;
                        break;
                    default: return nil;
                }
                return [NSString stringWithCharacters:&character length:1];
            }

            NSEventModifierFlags translate_modifiers(KeyModifierFlag modifiers)
            {
                NSEventModifierFlags flags = 0;
                u8 value = (u8)modifiers;
                if(value & (u8)KeyModifierFlag::ctrl) flags |= NSEventModifierFlagControl;
                if(value & (u8)KeyModifierFlag::shift) flags |= NSEventModifierFlagShift;
                if(value & (u8)KeyModifierFlag::alt) flags |= NSEventModifierFlagOption;
                if(value & (u8)KeyModifierFlag::system) flags |= NSEventModifierFlagCommand;
                return flags;
            }

            bool is_valid_modifier_flags(KeyModifierFlag modifiers)
            {
                constexpr u8 valid_flags = (u8)KeyModifierFlag::ctrl | (u8)KeyModifierFlag::shift |
                    (u8)KeyModifierFlag::alt | (u8)KeyModifierFlag::system;
                return (((u8)modifiers) & ~valid_flags) == 0;
            }

            RV validate_role(ApplicationMenuItemType type, ApplicationMenuItemRole role, ValidationContext& context)
            {
                switch(role)
                {
                    case ApplicationMenuItemRole::none:
                        return ok;
                    case ApplicationMenuItemRole::about:
                    case ApplicationMenuItemRole::settings:
                    case ApplicationMenuItemRole::hide:
                    case ApplicationMenuItemRole::hide_others:
                    case ApplicationMenuItemRole::show_all:
                    case ApplicationMenuItemRole::quit:
                        if(type != ApplicationMenuItemType::command)
                        {
                            return set_error(E_BAD_ARGUMENTS, "This application menu role requires a command item.");
                        }
                        return ok;
                    case ApplicationMenuItemRole::services:
                        if(type != ApplicationMenuItemType::submenu)
                        {
                            return set_error(E_BAD_ARGUMENTS, "The Services application menu role requires a submenu item.");
                        }
                        if(context.has_services_menu)
                        {
                            return set_error(E_BAD_ARGUMENTS, "Only one Services application menu is allowed.");
                        }
                        context.has_services_menu = true;
                        return ok;
                    case ApplicationMenuItemRole::window_menu:
                        if(type != ApplicationMenuItemType::submenu)
                        {
                            return set_error(E_BAD_ARGUMENTS, "The Window application menu role requires a submenu item.");
                        }
                        if(context.has_windows_menu)
                        {
                            return set_error(E_BAD_ARGUMENTS, "Only one Window application menu is allowed.");
                        }
                        context.has_windows_menu = true;
                        return ok;
                    case ApplicationMenuItemRole::help_menu:
                        if(type != ApplicationMenuItemType::submenu)
                        {
                            return set_error(E_BAD_ARGUMENTS, "The Help application menu role requires a submenu item.");
                        }
                        if(context.has_help_menu)
                        {
                            return set_error(E_BAD_ARGUMENTS, "Only one Help application menu is allowed.");
                        }
                        context.has_help_menu = true;
                        return ok;
                    default:
                        return set_error(E_BAD_ARGUMENTS, "The application menu item has an invalid role.");
                }
            }

            RV validate_items(Span<const ApplicationMenuItemDesc> items, ValidationContext& context,
                usize depth, bool top_level)
            {
                if(depth > MAX_MENU_DEPTH)
                {
                    return set_error(E_BAD_ARGUMENTS, "The application menu tree is too deeply nested or cyclic.");
                }
                for(const ApplicationMenuItemDesc& item : items)
                {
                    switch(item.type)
                    {
                        case ApplicationMenuItemType::command:
                        case ApplicationMenuItemType::submenu:
                        case ApplicationMenuItemType::separator:
                            break;
                        default:
                            return set_error(E_BAD_ARGUMENTS, "The application menu item has an invalid type.");
                    }
                    if(top_level && item.type != ApplicationMenuItemType::submenu)
                    {
                        return set_error(E_BAD_ARGUMENTS, "Every top-level application menu item must be a submenu.");
                    }
                    if(item.type != ApplicationMenuItemType::submenu && !item.children.empty())
                    {
                        return set_error(E_BAD_ARGUMENTS, "Only submenu items may contain child menu items.");
                    }
                    if(item.type == ApplicationMenuItemType::separator && item.role != ApplicationMenuItemRole::none)
                    {
                        return set_error(E_BAD_ARGUMENTS, "Application menu separators cannot have a standard role.");
                    }
                    if(item.type == ApplicationMenuItemType::command && item.role == ApplicationMenuItemRole::none && !item.id)
                    {
                        return set_error(E_BAD_ARGUMENTS, "An application-defined menu command must have a non-zero identifier.");
                    }
                    if(item.type == ApplicationMenuItemType::command && item.role == ApplicationMenuItemRole::settings && !item.id)
                    {
                        return set_error(E_BAD_ARGUMENTS, "A Settings menu command must have a non-zero identifier.");
                    }
                    if(item.id && !context.ids.insert(item.id).second)
                    {
                        return set_error(E_BAD_ARGUMENTS, "Application menu item identifiers must be unique.");
                    }
                    if(item.type != ApplicationMenuItemType::separator)
                    {
                        if(item.title)
                        {
                            if(!string_from_utf8(item.title))
                            {
                                return set_error(E_BAD_ARGUMENTS, "An application menu item title is not valid UTF-8.");
                            }
                        }
                        else if(item.role == ApplicationMenuItemRole::none)
                        {
                            return set_error(E_BAD_ARGUMENTS, "An application-defined menu item must have a title.");
                        }
                    }
                    RV role_result = validate_role(item.type, item.role, context);
                    if(failed(role_result)) return role_result;
                    if(!is_valid_modifier_flags(item.shortcut_modifiers))
                    {
                        return set_error(E_BAD_ARGUMENTS, "The application menu shortcut has invalid modifier flags.");
                    }
                    switch(item.state.check_state)
                    {
                        case ApplicationMenuItemCheckState::none:
                        case ApplicationMenuItemCheckState::checked:
                        case ApplicationMenuItemCheckState::mixed:
                            break;
                        default:
                            return set_error(E_BAD_ARGUMENTS, "The application menu item has an invalid check state.");
                    }
                    if(item.shortcut_key != KeyCode::unknown)
                    {
                        if(item.type != ApplicationMenuItemType::command)
                        {
                            return set_error(E_BAD_ARGUMENTS, "Only application menu commands may have keyboard shortcuts.");
                        }
                        NSEventModifierFlags additional_modifiers = 0;
                        if(!key_equivalent_for_key(item.shortcut_key, additional_modifiers))
                        {
                            return set_error(E_BAD_ARGUMENTS, "The application menu shortcut key is not supported on macOS.");
                        }
                    }
                    if(item.type == ApplicationMenuItemType::submenu)
                    {
                        RV child_result = validate_items(item.children, context, depth + 1, false);
                        if(failed(child_result)) return child_result;
                    }
                }
                return ok;
            }

            NSControlStateValue translate_check_state(ApplicationMenuItemCheckState state)
            {
                switch(state)
                {
                    case ApplicationMenuItemCheckState::checked: return NSControlStateValueOn;
                    case ApplicationMenuItemCheckState::mixed: return NSControlStateValueMixed;
                    default: return NSControlStateValueOff;
                }
            }

            void apply_item_state(NSMenuItem* item, const ApplicationMenuItemState& state)
            {
                objc_setAssociatedObject(item, &g_item_enabled_state_key,
                    [NSNumber numberWithBool:state.enabled ? YES : NO], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                [item setEnabled:state.enabled ? YES : NO];
                [item setState:translate_check_state(state.check_state)];
                [item setHidden:state.visible ? NO : YES];
            }

            void get_command_action(ApplicationMenuItemRole role, SEL* action, id __strong* target)
            {
                switch(role)
                {
                    case ApplicationMenuItemRole::none:
                        *action = @selector(invokeApplicationMenuItem:);
                        *target = g_application_menu_action_target;
                        break;
                    case ApplicationMenuItemRole::about:
                        *action = @selector(orderFrontStandardAboutPanel:);
                        *target = NSApp;
                        break;
                    case ApplicationMenuItemRole::settings:
                        *action = @selector(invokeApplicationMenuItem:);
                        *target = g_application_menu_action_target;
                        break;
                    case ApplicationMenuItemRole::hide:
                        *action = @selector(hide:);
                        *target = NSApp;
                        break;
                    case ApplicationMenuItemRole::hide_others:
                        *action = @selector(hideOtherApplications:);
                        *target = NSApp;
                        break;
                    case ApplicationMenuItemRole::show_all:
                        *action = @selector(unhideAllApplications:);
                        *target = NSApp;
                        break;
                    case ApplicationMenuItemRole::quit:
                        *action = @selector(requestApplicationQuit:);
                        *target = g_application_menu_action_target;
                        break;
                    default:
                        *action = nil;
                        *target = nil;
                        break;
                }
            }

            RV build_items(NSMenu* menu, Span<const ApplicationMenuItemDesc> items, BuiltMenu& built)
            {
                for(const ApplicationMenuItemDesc& desc : items)
                {
                    NSMenuItem* item = nil;
                    if(desc.type == ApplicationMenuItemType::separator)
                    {
                        item = [NSMenuItem separatorItem];
                        if(!item) return set_error(E_OUT_OF_MEMORY, "Failed to allocate an application menu separator.");
                    }
                    else
                    {
                        NSString* title = title_for_item(desc);
                        if(!title) return set_error(E_OUT_OF_MEMORY, "Failed to create an application menu item title.");

                        SEL action = nil;
                        id target = nil;
                        if(desc.type == ApplicationMenuItemType::command)
                        {
                            get_command_action(desc.role, &action, &target);
                        }
                        NSEventModifierFlags additional_modifiers = 0;
                        NSString* key_equivalent = @"";
                        if(desc.shortcut_key != KeyCode::unknown)
                        {
                            key_equivalent = key_equivalent_for_key(desc.shortcut_key, additional_modifiers);
                        }
                        item = [[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:key_equivalent];
                        if(!item) return set_error(E_OUT_OF_MEMORY, "Failed to allocate an application menu item.");
                        [item setTarget:target];
                        [item setKeyEquivalentModifierMask:translate_modifiers(desc.shortcut_modifiers) | additional_modifiers];

                        if(desc.type == ApplicationMenuItemType::submenu)
                        {
                            NSMenu* submenu = [[NSMenu alloc] initWithTitle:title];
                            if(!submenu) return set_error(E_OUT_OF_MEMORY, "Failed to allocate an application submenu.");
                            if(desc.role != ApplicationMenuItemRole::services &&
                                desc.role != ApplicationMenuItemRole::window_menu &&
                                desc.role != ApplicationMenuItemRole::help_menu)
                            {
                                // The descriptor state remains authoritative for SDK-managed menus. AppKit-owned
                                // Services, Window and Help menus keep automatic validation for their native items.
                                [submenu setAutoenablesItems:NO];
                            }
                            RV child_result = build_items(submenu, desc.children, built);
                            if(failed(child_result)) return child_result;
                            [item setSubmenu:submenu];
                            switch(desc.role)
                            {
                                case ApplicationMenuItemRole::services: built.services_menu = submenu; break;
                                case ApplicationMenuItemRole::window_menu: built.windows_menu = submenu; break;
                                case ApplicationMenuItemRole::help_menu: built.help_menu = submenu; break;
                                default: break;
                            }
                        }
                    }

                    apply_item_state(item, desc.state);
                    if(desc.id)
                    {
                        NSNumber* identifier = [NSNumber numberWithUnsignedLongLong:desc.id];
                        if(!identifier) return set_error(E_OUT_OF_MEMORY, "Failed to allocate an application menu item identifier.");
                        [built.items_by_id setObject:item forKey:identifier];
                        if(desc.type == ApplicationMenuItemType::command &&
                            (desc.role == ApplicationMenuItemRole::none || desc.role == ApplicationMenuItemRole::settings))
                        {
                            [item setRepresentedObject:identifier];
                        }
                    }
                    [menu addItem:item];
                }
                return ok;
            }

            RV build_menu(const ApplicationMenuDesc& desc, BuiltMenu& built)
            {
                built.main_menu = [[NSMenu alloc] initWithTitle:@""];
                built.items_by_id = [[NSMutableDictionary alloc] init];
                if(!built.main_menu || !built.items_by_id)
                {
                    return set_error(E_OUT_OF_MEMORY, "Failed to allocate the application main menu.");
                }
                [built.main_menu setAutoenablesItems:NO];
                return build_items(built.main_menu, desc.items, built);
            }

            bool ensure_luna_application_objects()
            {
                if(!g_application_menu_action_target)
                {
                    g_application_menu_action_target = [[LunaApplicationMenuActionTarget alloc] init];
                }
                if(!g_application_delegate)
                {
                    g_application_delegate = [[LunaApplicationDelegate alloc] init];
                }
                if(!g_application_menu_action_target || !g_application_delegate) return false;

                id<NSApplicationDelegate> current_delegate = [NSApp delegate];
                if(!current_delegate)
                {
                    [NSApp setDelegate:g_application_delegate];
                    g_application_delegate_attached = true;
                }
                else
                {
                    g_application_delegate_attached = current_delegate == g_application_delegate;
                }
                return true;
            }

            void install_built_menu(BuiltMenu& built, bool is_default)
            {
                [NSApp setMainMenu:built.main_menu];
                [NSApp setServicesMenu:built.services_menu];
                [NSApp setWindowsMenu:built.windows_menu];
                [NSApp setHelpMenu:built.help_menu];
                g_owned_main_menu = built.main_menu;
                g_owned_services_menu = built.services_menu;
                g_owned_windows_menu = built.windows_menu;
                g_owned_help_menu = built.help_menu;
                g_items_by_id = built.items_by_id;
                g_using_default_menu = is_default;
            }

            bool build_default_menu(BuiltMenu& built)
            {
                NSString* app_name = current_application_name();
                if(!app_name) return false;
                built.main_menu = [[NSMenu alloc] initWithTitle:@""];
                built.items_by_id = [[NSMutableDictionary alloc] init];
                if(!built.main_menu || !built.items_by_id) return false;
                [built.main_menu setAutoenablesItems:NO];

                NSMenuItem* app_menu_item = [[NSMenuItem alloc] initWithTitle:app_name action:nil keyEquivalent:@""];
                NSMenu* app_menu = [[NSMenu alloc] initWithTitle:app_name];
                if(!app_menu_item || !app_menu) return false;
                [app_menu setAutoenablesItems:NO];
                [app_menu_item setSubmenu:app_menu];
                [built.main_menu addItem:app_menu_item];

                NSString* about_title = [NSString stringWithFormat:@"About %@", app_name];
                if(!about_title) return false;
                NSMenuItem* about_item = [[NSMenuItem alloc] initWithTitle:about_title
                    action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
                if(!about_item) return false;
                [about_item setTarget:NSApp];
                [app_menu addItem:about_item];
                NSMenuItem* separator = [NSMenuItem separatorItem];
                if(!separator) return false;
                [app_menu addItem:separator];

                NSMenuItem* services_item = [[NSMenuItem alloc] initWithTitle:@"Services" action:nil keyEquivalent:@""];
                built.services_menu = [[NSMenu alloc] initWithTitle:@"Services"];
                if(!services_item || !built.services_menu) return false;
                [services_item setSubmenu:built.services_menu];
                [app_menu addItem:services_item];
                separator = [NSMenuItem separatorItem];
                if(!separator) return false;
                [app_menu addItem:separator];

                NSString* hide_title = [NSString stringWithFormat:@"Hide %@", app_name];
                if(!hide_title) return false;
                NSMenuItem* hide_item = [[NSMenuItem alloc] initWithTitle:hide_title action:@selector(hide:) keyEquivalent:@"h"];
                if(!hide_item) return false;
                [hide_item setTarget:NSApp];
                [hide_item setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
                [app_menu addItem:hide_item];

                NSMenuItem* hide_others_item = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
                    action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
                if(!hide_others_item) return false;
                [hide_others_item setTarget:NSApp];
                [hide_others_item setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagOption];
                [app_menu addItem:hide_others_item];

                NSMenuItem* show_all_item = [[NSMenuItem alloc] initWithTitle:@"Show All"
                    action:@selector(unhideAllApplications:) keyEquivalent:@""];
                if(!show_all_item) return false;
                [show_all_item setTarget:NSApp];
                [app_menu addItem:show_all_item];
                separator = [NSMenuItem separatorItem];
                if(!separator) return false;
                [app_menu addItem:separator];

                NSString* quit_title = [NSString stringWithFormat:@"Quit %@", app_name];
                if(!quit_title) return false;
                NSMenuItem* quit_item = [[NSMenuItem alloc] initWithTitle:quit_title
                    action:@selector(requestApplicationQuit:) keyEquivalent:@"q"];
                if(!quit_item) return false;
                [quit_item setTarget:g_application_menu_action_target];
                [quit_item setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
                [app_menu addItem:quit_item];
                return true;
            }

            RV install_default_menu()
            {
                BuiltMenu built;
                if(!build_default_menu(built))
                {
                    return set_error(E_OUT_OF_MEMORY, "Failed to allocate the default macOS application menu.");
                }
                install_built_menu(built, true);
                return ok;
            }

            bool action_target_can_invoke(NSMenuItem* item)
            {
                NSNumber* enabled = objc_getAssociatedObject(item, &g_item_enabled_state_key);
                return g_runtime_bridge_enabled && (!enabled || [enabled boolValue]) && ![item isHidden];
            }

            void invoke_application_menu_item(NSMenuItem* sender)
            {
                assert_cocoa_main_thread();
                if(!g_runtime_bridge_enabled) return;
                id represented_object = [sender representedObject];
                if(![represented_object isKindOfClass:[NSNumber class]]) return;
                application_menu_item_id_t item_id = [(NSNumber*)represented_object unsignedLongLongValue];
                if(item_id) dispatch_application_menu_item_invoked(item_id);
            }

            bool request_application_quit()
            {
                assert_cocoa_main_thread();
                if(!g_runtime_bridge_enabled) return false;
                dispatch_application_quit_request();
                return true;
            }
        }
    }
}

@implementation LunaApplicationDelegate

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    if(!Luna::Window::request_application_quit()) return NSTerminateNow;
    // Luna applications return through luna_main to release Runtime-owned resources.
    return NSTerminateCancel;
}

@end

@implementation LunaApplicationMenuActionTarget

- (void)invokeApplicationMenuItem:(NSMenuItem*)sender
{
    Luna::Window::invoke_application_menu_item(sender);
}

- (void)requestApplicationQuit:(NSMenuItem*)sender
{
    Luna::Window::request_application_quit();
}

- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
    return Luna::Window::action_target_can_invoke(menuItem) ? YES : NO;
}

@end


namespace Luna
{
    namespace Window
    {
        void cocoa_application_menu_bootstrap()
        {
            assert_cocoa_main_thread();
            [NSApplication sharedApplication];
            if(!ensure_luna_application_objects()) return;
            current_application_name();
            if(![NSApp mainMenu])
            {
                BuiltMenu built;
                if(build_default_menu(built)) install_built_menu(built, true);
            }
        }

        RV cocoa_application_menu_platform_init(const c8* app_name)
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                [NSApplication sharedApplication];
                if(!ensure_luna_application_objects())
                {
                    cocoa_application_menu_platform_close();
                    return set_error(E_OUT_OF_MEMORY, "Failed to allocate Cocoa application objects.");
                }

                NSString* configured_name = nil;
                if(app_name && app_name[0])
                {
                    configured_name = string_from_utf8(app_name);
                }
                if(configured_name && [configured_name length])
                {
                    g_application_name = [configured_name copy];
                }
                else if(!g_application_name || ![g_application_name length])
                {
                    g_application_name = [fallback_application_name() copy];
                }

                RV result = ok;
                if(![NSApp mainMenu])
                {
                    result = install_default_menu();
                }
                else if([NSApp mainMenu] == g_owned_main_menu && g_using_default_menu)
                {
                    result = install_default_menu();
                }
                if(failed(result))
                {
                    cocoa_application_menu_platform_close();
                    return result;
                }
                g_runtime_bridge_enabled = true;
                return ok;
            }
        }

        void cocoa_application_menu_platform_close()
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                g_runtime_bridge_enabled = false;
                memset(g_suppressed_key_ups, 0, sizeof(g_suppressed_key_ups));
                if([NSApp servicesMenu] == g_owned_services_menu) [NSApp setServicesMenu:nil];
                if([NSApp windowsMenu] == g_owned_windows_menu) [NSApp setWindowsMenu:nil];
                if([NSApp helpMenu] == g_owned_help_menu) [NSApp setHelpMenu:nil];
                if([NSApp mainMenu] == g_owned_main_menu) [NSApp setMainMenu:nil];
                if(g_application_delegate_attached && [NSApp delegate] == g_application_delegate)
                {
                    [NSApp setDelegate:nil];
                }
                g_application_delegate_attached = false;
                g_owned_main_menu = nil;
                g_owned_services_menu = nil;
                g_owned_windows_menu = nil;
                g_owned_help_menu = nil;
                g_items_by_id = nil;
                g_application_name = nil;
                g_application_delegate = nil;
                g_application_menu_action_target = nil;
                g_using_default_menu = false;
            }
        }

        bool cocoa_application_menu_handle_key_down(NSEvent* event)
        {
            lutsassert_main_thread();
            if(!event || [event type] != NSEventTypeKeyDown) return false;
            usize key_code = (usize)[event keyCode];
            // A menu action such as Hide may move focus before its key-up arrives. A new key-down for the
            // same physical key starts a new pair and must not inherit stale suppression from the old one.
            g_suppressed_key_ups[key_code] = false;
            NSMenu* menu = [NSApp mainMenu];
            if(!menu || ![menu performKeyEquivalent:event]) return false;
            g_suppressed_key_ups[key_code] = true;
            return true;
        }

        bool cocoa_application_menu_handle_key_up(NSEvent* event)
        {
            lutsassert_main_thread();
            if(!event || [event type] != NSEventTypeKeyUp) return false;
            usize key_code = (usize)[event keyCode];
            bool suppress = g_suppressed_key_ups[key_code];
            g_suppressed_key_ups[key_code] = false;
            return suppress;
        }

        LUNA_WINDOW_API bool supports_application_menu()
        {
            return true;
        }

        LUNA_WINDOW_API RV set_application_menu(const ApplicationMenuDesc& desc)
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                ValidationContext context;
                RV validation_result = validate_items(desc.items, context, 0, true);
                if(failed(validation_result)) return validation_result;

                BuiltMenu built;
                RV build_result = build_menu(desc, built);
                if(failed(build_result)) return build_result;
                install_built_menu(built, false);
                return ok;
            }
        }

        LUNA_WINDOW_API RV reset_application_menu()
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                return install_default_menu();
            }
        }

        LUNA_WINDOW_API RV set_application_menu_item_state(application_menu_item_id_t id,
            const ApplicationMenuItemState& state)
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                switch(state.check_state)
                {
                    case ApplicationMenuItemCheckState::none:
                    case ApplicationMenuItemCheckState::checked:
                    case ApplicationMenuItemCheckState::mixed:
                        break;
                    default: return E_BAD_ARGUMENTS;
                }
                if(!id || !g_items_by_id) return E_NOT_FOUND;
                NSMenuItem* item = [g_items_by_id objectForKey:[NSNumber numberWithUnsignedLongLong:id]];
                if(!item) return E_NOT_FOUND;
                apply_item_state(item, state);
                return ok;
            }
        }

        LUNA_WINDOW_API RV set_application_menu_item_title(application_menu_item_id_t id, const c8* title)
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                if(!title) return E_BAD_ARGUMENTS;
                NSString* native_title = string_from_utf8(title);
                if(!native_title) return E_BAD_ARGUMENTS;
                if(!id || !g_items_by_id) return E_NOT_FOUND;
                NSMenuItem* item = [g_items_by_id objectForKey:[NSNumber numberWithUnsignedLongLong:id]];
                if(!item) return E_NOT_FOUND;
                [item setTitle:native_title];
                if([item submenu]) [[item submenu] setTitle:native_title];
                return ok;
            }
        }
    }
}
