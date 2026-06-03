/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIWindow.hpp
* @author JXMaster
* @date 2026/5/21
*/
#pragma once
#include <Luna/GUI/GUI.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Window/Window.hpp>

#ifndef LUNA_GUI_WINDOW_API
#define LUNA_GUI_WINDOW_API
#endif

namespace Luna
{
    namespace GUIWindow
    {
        //! Bridges Window events into one GUI context.
        struct GUIWindowInputAdapter
        {
            Ref<Window::IWindow> window;
            Ref<GUI::IContext> gui;
            void(*next_event_handler)(object_t event, void* userdata) = nullptr;
            void* next_event_userdata = nullptr;
            bool forward_events = true;
        };

        //! Translates one Window event and routes it to the specified GUI context.
        //! @return Returns `true` if the event is translated to a GUI input event.
        LUNA_GUI_WINDOW_API bool handle_window_event(object_t event, Window::IWindow* window, GUI::IContext* gui);

        //! Installs a Window event handler that routes matching events to the adapter GUI context.
        LUNA_GUI_WINDOW_API void install_window_event_handler(GUIWindowInputAdapter* adapter);

        //! Restores the event handler that was active before @ref install_window_event_handler.
        LUNA_GUI_WINDOW_API void uninstall_window_event_handler(GUIWindowInputAdapter* adapter);

        //! Synchronizes per-frame window IO state with GUI, including pointer position and text input/IME state.
        //! Call this after IContext::submit().
        LUNA_GUI_WINDOW_API RV update_text_input(Window::IWindow* window, GUI::IContext* gui);

        //! Synchronizes per-frame adapter window IO state with GUI, including pointer position and text input/IME state.
        //! Call this after IContext::submit().
        LUNA_GUI_WINDOW_API RV update_text_input(GUIWindowInputAdapter* adapter);

        LUNA_GUI_WINDOW_API Module* module_gui_window();
    }
}
