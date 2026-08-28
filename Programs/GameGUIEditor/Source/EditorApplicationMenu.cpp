/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorApplicationMenu.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#if defined(LUNA_PLATFORM_MACOS)
#include <Luna/Window/ApplicationMenu.hpp>
#endif

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
#if defined(LUNA_PLATFORM_MACOS)
            constexpr Window::application_menu_item_id_t MENU_ITEM_NEW = 1;
            constexpr Window::application_menu_item_id_t MENU_ITEM_OPEN = 2;
            constexpr Window::application_menu_item_id_t MENU_ITEM_SAVE = 3;
            constexpr Window::application_menu_item_id_t MENU_ITEM_SAVE_AS = 4;
            constexpr Window::application_menu_item_id_t MENU_ITEM_CLOSE = 5;
            constexpr Window::application_menu_item_id_t MENU_ITEM_UNDO = 6;
            constexpr Window::application_menu_item_id_t MENU_ITEM_REDO = 7;

            Window::ApplicationMenuItemDesc menu_command(const c8* title,
                Window::application_menu_item_id_t id, KeyCode shortcut_key = KeyCode::unknown,
                Window::KeyModifierFlag shortcut_modifiers = Window::KeyModifierFlag::none)
            {
                Window::ApplicationMenuItemDesc desc;
                desc.title = title;
                desc.id = id;
                desc.shortcut_key = shortcut_key;
                desc.shortcut_modifiers = shortcut_modifiers;
                return desc;
            }

            Window::ApplicationMenuItemDesc standard_menu_command(Window::ApplicationMenuItemRole role,
                KeyCode shortcut_key = KeyCode::unknown,
                Window::KeyModifierFlag shortcut_modifiers = Window::KeyModifierFlag::none)
            {
                Window::ApplicationMenuItemDesc desc;
                desc.role = role;
                desc.shortcut_key = shortcut_key;
                desc.shortcut_modifiers = shortcut_modifiers;
                return desc;
            }

            Window::ApplicationMenuItemDesc menu_separator()
            {
                Window::ApplicationMenuItemDesc desc;
                desc.type = Window::ApplicationMenuItemType::separator;
                return desc;
            }

            Window::ApplicationMenuItemDesc menu_submenu(const c8* title,
                Span<const Window::ApplicationMenuItemDesc> children,
                Window::ApplicationMenuItemRole role = Window::ApplicationMenuItemRole::none)
            {
                Window::ApplicationMenuItemDesc desc;
                desc.type = Window::ApplicationMenuItemType::submenu;
                desc.role = role;
                desc.title = title;
                desc.children = children;
                return desc;
            }

            Window::ApplicationMenuItemState menu_item_state(bool enabled)
            {
                Window::ApplicationMenuItemState state;
                state.enabled = enabled;
                return state;
            }
#endif
#if defined(LUNA_PLATFORM_MACOS)
            RV EditorApp::install_application_menu()
            {
                Window::ApplicationMenuItemDesc app_items[] =
                {
                    standard_menu_command(Window::ApplicationMenuItemRole::about),
                    menu_separator(),
                    menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::services),
                    menu_separator(),
                    standard_menu_command(Window::ApplicationMenuItemRole::hide,
                        KeyCode::h, Window::KeyModifierFlag::system),
                    standard_menu_command(Window::ApplicationMenuItemRole::hide_others,
                        KeyCode::h, Window::KeyModifierFlag::system | Window::KeyModifierFlag::alt),
                    standard_menu_command(Window::ApplicationMenuItemRole::show_all),
                    menu_separator(),
                    standard_menu_command(Window::ApplicationMenuItemRole::quit,
                        KeyCode::q, Window::KeyModifierFlag::system),
                };

                Window::ApplicationMenuItemDesc file_items[] =
                {
                    menu_command("New", MENU_ITEM_NEW, KeyCode::n, Window::KeyModifierFlag::system),
                    menu_command("Open...", MENU_ITEM_OPEN, KeyCode::o, Window::KeyModifierFlag::system),
                    menu_separator(),
                    menu_command("Save", MENU_ITEM_SAVE, KeyCode::s, Window::KeyModifierFlag::system),
                    menu_command("Save As...", MENU_ITEM_SAVE_AS, KeyCode::s,
                        Window::KeyModifierFlag::system | Window::KeyModifierFlag::shift),
                    menu_separator(),
                    menu_command("Close", MENU_ITEM_CLOSE, KeyCode::w, Window::KeyModifierFlag::system),
                };
                DocumentView* document = active_document();
                bool has_document = document != nullptr;
                file_items[3].state = menu_item_state(has_document);
                file_items[4].state = menu_item_state(has_document);
                file_items[6].state = menu_item_state(has_document);

                Window::ApplicationMenuItemDesc edit_items[] =
                {
                    menu_command("Undo", MENU_ITEM_UNDO, KeyCode::z, Window::KeyModifierFlag::system),
                    menu_command("Redo", MENU_ITEM_REDO, KeyCode::z,
                        Window::KeyModifierFlag::system | Window::KeyModifierFlag::shift),
                };
                edit_items[0].state = menu_item_state(document && document->can_undo);
                edit_items[1].state = menu_item_state(document && document->can_redo);

                Window::ApplicationMenuItemDesc main_items[] =
                {
                    menu_submenu(APP_NAME, Span<const Window::ApplicationMenuItemDesc>(app_items, 9)),
                    menu_submenu("File", Span<const Window::ApplicationMenuItemDesc>(file_items, 7)),
                    menu_submenu("Edit", Span<const Window::ApplicationMenuItemDesc>(edit_items, 2)),
                    menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::window_menu),
                    menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::help_menu),
                };
                Window::ApplicationMenuDesc desc;
                desc.items = Span<const Window::ApplicationMenuItemDesc>(main_items, 5);
                lutry
                {
                    luexp(Window::set_application_menu(desc));
                    application_menu_has_document = has_document;
                    application_menu_can_undo = document && document->can_undo;
                    application_menu_can_redo = document && document->can_redo;
                }
                lucatchret;
                return ok;
            }

            RV EditorApp::update_application_menu_state()
            {
                DocumentView* document = active_document();
                bool has_document = document != nullptr;
                bool can_undo = document && document->can_undo;
                bool can_redo = document && document->can_redo;
                lutry
                {
                    if(has_document != application_menu_has_document)
                    {
                        Window::ApplicationMenuItemState state = menu_item_state(has_document);
                        luexp(Window::set_application_menu_item_state(MENU_ITEM_SAVE, state));
                        luexp(Window::set_application_menu_item_state(MENU_ITEM_SAVE_AS, state));
                        luexp(Window::set_application_menu_item_state(MENU_ITEM_CLOSE, state));
                        application_menu_has_document = has_document;
                    }
                    if(can_undo != application_menu_can_undo)
                    {
                        luexp(Window::set_application_menu_item_state(MENU_ITEM_UNDO,
                            menu_item_state(can_undo)));
                        application_menu_can_undo = can_undo;
                    }
                    if(can_redo != application_menu_can_redo)
                    {
                        luexp(Window::set_application_menu_item_state(MENU_ITEM_REDO,
                            menu_item_state(can_redo)));
                        application_menu_can_redo = can_redo;
                    }
                }
                lucatchret;
                return ok;
            }

            void EditorApp::handle_application_menu_item(Window::application_menu_item_id_t id)
            {
                switch(id)
                {
                case MENU_ITEM_NEW:
                    create_document();
                    break;
                case MENU_ITEM_OPEN:
                    open_document();
                    break;
                case MENU_ITEM_SAVE:
                {
                    DocumentView* document = active_document();
                    if(document) save(*document, false);
                    break;
                }
                case MENU_ITEM_SAVE_AS:
                {
                    DocumentView* document = active_document();
                    if(document) save(*document, true);
                    break;
                }
                case MENU_ITEM_CLOSE:
                {
                    DocumentView* document = active_document();
                    if(document) request_close(*document, false);
                    break;
                }
                case MENU_ITEM_UNDO:
                {
                    DocumentView* document = active_document();
                    if(document) undo_document(*document);
                    break;
                }
                case MENU_ITEM_REDO:
                {
                    DocumentView* document = active_document();
                    if(document) redo_document(*document);
                    break;
                }
                default:
                    break;
                }
            }
#endif
        }
    }
}
