/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.cpp
* @author JXMaster
* @date 2025/10/7
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../Event.hpp"
#include "Window.hpp"
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/SpinLock.hpp>

namespace Luna
{
    namespace Window
    {
        RingDeque<ObjRef> g_event_queue;
        SpinLock g_event_queue_lock;

        void init_events()
        {
            register_struct_type<WindowRequestCloseEvent>({});
            register_struct_type<WindowClosedEvent>({});
            register_struct_type<WindowInputFocusEvent>({});
            register_struct_type<WindowLoseInputFocusEvent>({});
            register_struct_type<WindowShowEvent>({});
            register_struct_type<WindowHideEvent>({});
            register_struct_type<WindowResizeEvent>({});
            register_struct_type<WindowFramebufferResizeEvent>({});
            register_struct_type<WindowMoveEvent>({});
            register_struct_type<WindowDPIScaleChangedEvent>({});
            register_struct_type<WindowKeyDownEvent>({});
            register_struct_type<WindowKeyUpEvent>({});
            register_struct_type<WindowInputTextEvent>({});
            register_struct_type<WindowMouseEnterEvent>({});
            register_struct_type<WindowMouseLeaveEvent>({});
            register_struct_type<WindowMouseMoveEvent>({});
            register_struct_type<WindowMouseDownEvent>({});
            register_struct_type<WindowMouseUpEvent>({});
            register_struct_type<WindowScrollEvent>({});
            register_struct_type<WindowTouchDownEvent>({});
            register_struct_type<WindowTouchMoveEvent>({});
            register_struct_type<WindowTouchUpEvent>({});
            register_struct_type<WindowDropFilesEvent>({});
        }
        void close_events()
        {
            g_event_queue.clear();
            g_event_queue.shrink_to_fit();
        }
        LUNA_WINDOW_API ObjRef pop_event(bool wait_event)
        {
            platform_poll_events(wait_event);
            LockGuard guard(g_event_queue_lock);
            // If wait_event is true, the event queue must has at least one event here.
            luassert(!wait_event || !g_event_queue.empty());
            if(g_event_queue.empty()) return ObjRef();
            ObjRef event = move(g_event_queue.front());
            g_event_queue.pop_front();
            return event;
        }
        LUNA_WINDOW_API void push_event(object_t event)
        {
            LockGuard guard(g_event_queue_lock);
            g_event_queue.push_back(ObjRef(event));
        }
        LUNA_WINDOW_API void default_event_handler(object_t event)
        {
            if(WindowRequestCloseEvent* e = cast_object<WindowRequestCloseEvent>(event))
            {
                e->window->close();
            }
        }
    }
}