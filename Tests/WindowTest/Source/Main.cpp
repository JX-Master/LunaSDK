/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2025/3/20
*/
#include <Luna/Window/Application.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/ApplicationMenu.hpp>
#include <Luna/Runtime/Log.hpp>

namespace Luna
{
    namespace
    {
        constexpr Window::application_menu_item_id_t MENU_ITEM_TOGGLE_RESIZABLE = 1;

        struct WindowTestContext
        {
            Ref<Window::IWindow> window;
            bool application_menu_supported = false;
        };

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

        RV install_test_menu()
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
            Window::ApplicationMenuItemDesc test_items[1];
            test_items[0].title = "Resizable";
            test_items[0].id = MENU_ITEM_TOGGLE_RESIZABLE;
            test_items[0].state.check_state = Window::ApplicationMenuItemCheckState::checked;
            test_items[0].shortcut_key = KeyCode::r;
            test_items[0].shortcut_modifiers = Window::KeyModifierFlag::system;

            Window::ApplicationMenuItemDesc main_items[] =
            {
                menu_submenu("Window Test", Span<const Window::ApplicationMenuItemDesc>(app_items, 9)),
                menu_submenu("Test", Span<const Window::ApplicationMenuItemDesc>(test_items, 1)),
                menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::window_menu),
                menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::help_menu),
            };
            Window::ApplicationMenuDesc desc;
            desc.items = Span<const Window::ApplicationMenuItemDesc>(main_items, 4);
            return Window::set_application_menu(desc);
        }

        void toggle_resizable(WindowTestContext* context)
        {
            if(!context || !context->window) return;
            auto style = context->window->get_style();
            set_flags(style, Window::WindowStyleFlag::resizable,
                !test_flags(style, Window::WindowStyleFlag::resizable));
            lupanic_if_failed(context->window->set_style(style));
            if(context->application_menu_supported)
            {
                Window::ApplicationMenuItemState state;
                state.check_state = test_flags(style, Window::WindowStyleFlag::resizable) ?
                    Window::ApplicationMenuItemCheckState::checked : Window::ApplicationMenuItemCheckState::none;
                lupanic_if_failed(Window::set_application_menu_item_state(MENU_ITEM_TOGGLE_RESIZABLE, state));
            }
        }

        void window_test_event_handler(object_t event, void* userdata)
        {
            WindowTestContext* context = (WindowTestContext*)userdata;
            if(auto e = cast_object<Window::WindowKeyDownEvent>(event))
            {
                if(e->key == KeyCode::r)
                {
                    toggle_resizable(context);
                }
            }
            if(auto e = cast_object<Window::ApplicationMenuItemInvokedEvent>(event))
            {
                if(e->item_id == MENU_ITEM_TOGGLE_RESIZABLE)
                {
                    toggle_resizable(context);
                }
            }
            if(auto e = cast_object<Window::ApplicationRequestQuitEvent>(event))
            {
                e->do_quit = true;
            }
        }
    }
}

int luna_main(int argc, const char* argv[])
{
    using namespace Luna;
    if(Luna::failed(Luna::init())) return -1;
    lutry
    {
        luexp(add_modules({module_window()}));
        Window::StartupParams params;
        params.name = "Window Test";
        Window::set_startup_params(params);
        luexp(init_modules());

        WindowTestContext context;
        context.application_menu_supported = Window::supports_application_menu();
        if(context.application_menu_supported)
        {
            luexp(install_test_menu());
        }
        luset(context.window, Window::new_window("Window Test"));
        Window::set_event_handler(window_test_event_handler, &context);

        while(true)
        {
            Window::poll_events();
            if(context.window->is_closed() || Window::is_application_quit_requested()) break;
            sleep(16);
        }
        Window::set_event_handler(nullptr, nullptr);
        if(context.application_menu_supported)
        {
            auto _ = Window::reset_application_menu();
        }
    }
    lucatch
    {
        log_error("WindowTest", "%s", explain(luerr));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}
