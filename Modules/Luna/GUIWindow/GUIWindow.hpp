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
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Window/Window.hpp>

#ifndef LUNA_GUI_WINDOW_API
#define LUNA_GUI_WINDOW_API
#endif

namespace Luna
{
    namespace GUIWindow
    {
        //! Bridges Window events into one GUI Core context.
        struct GUICoreWindowInputAdapter
        {
            //! Window whose events should be routed.
            Ref<Window::IWindow> window;
            //! GUI Core context that receives translated events.
            Ref<GUICore::IContext> gui;
            //! Window events translated before the target GUI Core frame begins.
            //! @remark Call @ref update_input after @ref GUICore::IContext::begin_frame to flush these events.
            Vector<GUICore::InputEvent> pending_events;
            //! Event handler active before this adapter was installed.
            void(*next_event_handler)(object_t event, void* userdata) = nullptr;
            //! User data for @ref next_event_handler.
            void* next_event_userdata = nullptr;
            //! Whether translated events should also be forwarded to @ref next_event_handler.
            bool forward_events = true;
        };

        //! Translates one Window event and routes it to the specified GUI Core context.
        //! @return Returns `true` if the event is translated to a GUI Core input event.
        LUNA_GUI_WINDOW_API bool handle_window_event(object_t event, Window::IWindow* window, GUICore::IContext* gui);

        //! Installs a Window event handler that routes matching events to the adapter GUI Core context.
        LUNA_GUI_WINDOW_API void install_window_event_handler(GUICoreWindowInputAdapter* adapter);

        //! Restores the event handler that was active before @ref install_window_event_handler.
        LUNA_GUI_WINDOW_API void uninstall_window_event_handler(GUICoreWindowInputAdapter* adapter);

        //! Synchronizes per-frame text input/IME state with GUI Core.
        //! @remark Call this after GUI Core elements have been built, laid out and routed for the frame.
        LUNA_GUI_WINDOW_API RV update_text_input(Window::IWindow* window, GUICore::IContext* gui);

        //! Synchronizes per-frame adapter text input/IME state with GUI Core.
        //! @remark Call this after GUI Core elements have been built, laid out and routed for the frame.
        LUNA_GUI_WINDOW_API RV update_text_input(GUICoreWindowInputAdapter* adapter);

        //! Synchronizes per-frame pointer state and clipboard callbacks with GUI Core.
        //! @remark Call this after @ref GUICore::IContext::begin_frame and before building the GUI Core frame.
        LUNA_GUI_WINDOW_API void update_input(Window::IWindow* window, GUICore::IContext* gui);

        //! Synchronizes per-frame adapter pointer state with GUI Core.
        LUNA_GUI_WINDOW_API void update_input(GUICoreWindowInputAdapter* adapter);

        LUNA_GUI_WINDOW_API Module* module_gui_window();
    }
}
