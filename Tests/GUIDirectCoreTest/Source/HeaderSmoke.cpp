/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HeaderSmoke.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/GUI/GUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>

namespace Luna
{
    namespace
    {
        void compile_default_gui_package_entry()
        {
            using ModuleFn = Module*(*)();
            using RegisterStyleSchemasFn = void(*)(GUICore::IContext*);
            using TextFn = GUICore::ElementHandle(*)(GUICore::IContext*, GUICore::id_t, const c8*, const GUICore::LayoutInput&);
            using TextButtonFn = GUICore::ElementHandle(*)(GUICore::IContext*, GUICore::id_t, const c8*, const GUICore::LayoutInput&, bool);
            using BeginLayoutFn = GUICore::ElementHandle(*)(GUICore::IContext*, GUICore::id_t, const c8*, const GUICore::LayoutInput&);
            using MakeStateIdFn = GUICore::id_t(*)(GUICore::id_t, const Guid&);
            using WindowInputAdapter = GUIWindow::GUICoreWindowInputAdapter;
            using WindowEventFn = bool(*)(object_t, Window::IWindow*, GUICore::IContext*);
            using WindowUpdateInputFn = void(*)(Window::IWindow*, GUICore::IContext*);
            using WindowUpdateTextInputFn = RV(*)(Window::IWindow*, GUICore::IContext*);

            ModuleFn module_fn = GUI::module_gui;
            RegisterStyleSchemasFn register_style_schemas_fn = GUI::register_editor_style_schemas;
            TextFn text_fn = GUI::text;
            TextButtonFn text_button_fn = GUI::text_button;
            BeginLayoutFn begin_h_layout_fn = GUI::begin_h_layout;
            MakeStateIdFn make_state_id_fn = GUICore::make_state_id;
            ModuleFn gui_window_module_fn = GUIWindow::module_gui_window;
            WindowInputAdapter* adapter = nullptr;
            WindowEventFn window_event_fn = GUIWindow::handle_window_event;
            WindowUpdateInputFn update_input_fn = GUIWindow::update_input;
            WindowUpdateTextInputFn update_text_input_fn = GUIWindow::update_text_input;

            (void)module_fn;
            (void)register_style_schemas_fn;
            (void)text_fn;
            (void)text_button_fn;
            (void)begin_h_layout_fn;
            (void)make_state_id_fn;
            (void)gui_window_module_fn;
            (void)adapter;
            (void)window_event_fn;
            (void)update_input_fn;
            (void)update_text_input_fn;
        }
    }
}
