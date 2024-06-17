/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2024/6/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include "Monitor.hpp"
#include <Luna/Runtime/Thread.hpp>

namespace Luna
{
    namespace Window
    {
        void Window::close()
        {
            
        }
        RV platform_init()
        {
            if (SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                return set_error(BasicError::bad_platform_call(), "SDL initialization failed: %s", SDL_GetError());
            }
            return monitor_init();
        }
        void platform_close()
        {
            monitor_close();
            SDL_Quit();
        }

        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            SDL_Event event;
            int any_event = 0;
            if(wait_events)
            {
                any_event = SDL_WaitEvent(&event);
            }
            else
            {
                any_event = SDL_PollEvent(&event);
            }
            if (any_event)
            {
                // Handle event.
                switch(event.type)
                {
                    case SDL_DISPLAYEVENT:
                    {
                        MonitorEvent e;
                        e.orientation = MonitorOrientation::unknown;
                        switch(event.display.event)
                        {
                            case SDL_DISPLAYEVENT_ORIENTATION:
                            e.type = MonitorEventType::orientation;
                            switch(event.display.data1)
                            {
                                case SDL_ORIENTATION_UNKNOWN:
                                e.orientation = MonitorOrientation::unknown;
                                break;
                                case SDL_ORIENTATION_LANDSCAPE:
                                e.orientation = MonitorOrientation::landscape;
                                break;
                                case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                                e.orientation = MonitorOrientation::landscape_flipped;
                                break;
                                case SDL_ORIENTATION_PORTRAIT:
                                e.orientation = MonitorOrientation::portrait;
                                break;
                                case SDL_ORIENTATION_PORTRAIT_FLIPPED:
                                e.orientation = MonitorOrientation::portrait_flipped;
                                break;
                            }
                            break;
                            case SDL_DISPLAYEVENT_CONNECTED:
                            e.type = MonitorEventType::connected;
                            break;
                            case SDL_DISPLAYEVENT_DISCONNECTED:
                            {
                                e.type = MonitorEventType::disconnected;
                                Monitor* m = (Monitor*)get_monitor(event.display.display);
                                m->m_disconnected = true;
                            }
                            break;
                            case SDL_DISPLAYEVENT_MOVED:
                            e.type = MonitorEventType::moved;
                            break;
                            default:
                            lupanic();
                            break;
                        }
                        if (e.type == MonitorEventType::connected)
                        {
                            lupanic_if_failed(refresh_monitor_list());
                        }
                        dispatch_monitor_event(get_monitor(event.display.display), e);
                        if (e.type == MonitorEventType::disconnected)
                        {
                            lupanic_if_failed(refresh_monitor_list());
                        }
                    }
                    break;
                    case SDL_WINDOWEVENT:
                    {

                    }
                }
            }
        }
        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, const WindowDisplaySettings& display_settings, WindowCreationFlag flags)
        {
            lucheck_msg(get_current_thread() == get_main_thread(), "RHI::new_window must only be called from the main thread.");
            Ref<Window> window = new_object<Window>();
            u32 window_flags = 0;
            int x = SDL_WINDOWPOS_UNDEFINED;
            int y = SDL_WINDOWPOS_UNDEFINED;
            int w = 0;
            int h = 0;
            if (display_settings.full_screen)
            {
                window_flags |= SDL_WINDOW_FULLSCREEN;
            }
            else
            {
                if (display_settings.x != DEFAULT_POS) x = display_settings.x;
                if (display_settings.y != DEFAULT_POS) y = display_settings.y;
            }
        }
    }
}