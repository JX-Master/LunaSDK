/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Monitor.cpp
* @author JXMaster
* @date 2024/6/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT

#include "Monitor.hpp"
#include <SDL.h>
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    namespace Window
    {
        Vector<UniquePtr<Monitor>> g_monitors;
        Event<monitor_event_handler_t> g_monitor_change_event;
        inline void set_video_mode_rgb_bits(VideoMode& mode, u32 rbits, u32 gbits, u32 bbits)
        {
            mode.red_bits = rbits;
            mode.green_bits = gbits;
            mode.blue_bits = bbits;
        }
        VideoMode encode_video_mode(const SDL_DisplayMode& mode)
        {
            VideoMode dst_mode;
            dst_mode.width = mode.w;
            dst_mode.height = mode.h;
            dst_mode.refresh_rate = mode.refresh_rate;
            switch (mode.format)
            {
                case SDL_PIXELFORMAT_RGB332:
                    set_video_mode_rgb_bits(dst_mode, 3, 3, 2);
                    break;
                case SDL_PIXELFORMAT_RGB444:
                case SDL_PIXELFORMAT_BGR444:
                case SDL_PIXELFORMAT_ARGB4444:
                case SDL_PIXELFORMAT_RGBA4444:
                case SDL_PIXELFORMAT_ABGR4444:
                case SDL_PIXELFORMAT_BGRA4444:
                    set_video_mode_rgb_bits(dst_mode, 4, 4, 4);
                    break;
                case SDL_PIXELFORMAT_RGB555:
                case SDL_PIXELFORMAT_BGR555:
                case SDL_PIXELFORMAT_ARGB1555:
                case SDL_PIXELFORMAT_RGBA5551:
                case SDL_PIXELFORMAT_ABGR1555:
                case SDL_PIXELFORMAT_BGRA5551:
                    set_video_mode_rgb_bits(dst_mode, 5, 5, 5);
                    break;
                case SDL_PIXELFORMAT_RGB565:
                case SDL_PIXELFORMAT_BGR565:
                    set_video_mode_rgb_bits(dst_mode, 5, 6, 5);
                    break;
                case SDL_PIXELFORMAT_RGB24:
                case SDL_PIXELFORMAT_BGR24:
                case SDL_PIXELFORMAT_RGB888:
                case SDL_PIXELFORMAT_RGBX8888:
                case SDL_PIXELFORMAT_BGR888:
                case SDL_PIXELFORMAT_BGRX8888:
                case SDL_PIXELFORMAT_ARGB8888:
                case SDL_PIXELFORMAT_RGBA8888:
                case SDL_PIXELFORMAT_ABGR8888:
                case SDL_PIXELFORMAT_BGRA8888:
                    set_video_mode_rgb_bits(dst_mode, 8, 8, 8);
                    break;
                case SDL_PIXELFORMAT_ARGB2101010:
                    set_video_mode_rgb_bits(dst_mode, 10, 10, 10);
                    break;
                default:
                    dst_mode.red_bits = 0;
                    dst_mode.green_bits = 0;
                    dst_mode.blue_bits = 0;
                    break;
            }
            return dst_mode;
        }
        RV refresh_monitor_list()
        {
            Vector<UniquePtr<Monitor>> old_monitors = move(g_monitors);
            int num_displays = SDL_GetNumVideoDisplays();
            if (num_displays < 0)
            {
                return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
            }
            g_monitors.reserve((u32)num_displays);
            for(int i = 0; i < num_displays; ++i)
            {
                Name monitor_name = SDL_GetDisplayName(i);
                // try to match existing monitor.
                UniquePtr<Monitor> monitor;
                for(auto iter = old_monitors.begin(); iter != old_monitors.end(); ++iter)
                {
                    if (iter->get()->m_name == monitor_name && !iter->get()->m_disconnected)
                    {
                        monitor = move(*iter);
                        old_monitors.erase(iter);
                        break;
                    }
                }
                // Freate new monitor instance if not matched.
                if (!monitor)
                {
                    monitor.reset(memnew<Monitor>());
                }
                // Fill monitor data.
                monitor->m_name = monitor_name;
                monitor->m_index = i;
                g_monitors.push_back(move(monitor));
            }
            return ok;
        }
        RV monitor_init()
        {
            return refresh_monitor_list();
        }
        void monitor_close()
        {
            g_monitors.clear();
            g_monitors.shrink_to_fit();
            g_monitor_change_event.clear();
        }
        LUNA_WINDOW_API monitor_t get_primary_monitor()
        {
            //! The primary display is always listed at #0.
            return get_monitor(0);
        }
        LUNA_WINDOW_API u32 count_monitors()
        {
            return (u32)g_monitors.size();
        }
        LUNA_WINDOW_API monitor_t get_monitor(u32 index)
        {
            return (monitor_t)g_monitors[index].get();
        }
        LUNA_WINDOW_API Event<monitor_event_handler_t>& get_monitor_event()
        {
            return g_monitor_change_event;
        }
        void dispatch_monitor_event(monitor_t monitor, const MonitorEvent& e)
        {
            g_monitor_change_event(monitor, e);
        }
        LUNA_WINDOW_API u32 count_monitor_supported_video_modes(monitor_t monitor)
        {
            Monitor* m = (Monitor*)monitor;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected monitor.");
            return (u32)SDL_GetNumDisplayModes(m->m_index);
        }
        LUNA_WINDOW_API VideoMode get_monitor_supported_video_mode(monitor_t monitor, u32 index)
        {
            Monitor* m = (Monitor*)monitor;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected monitor.");
            SDL_DisplayMode mode;
            int r = SDL_GetDisplayMode(m->m_index, index, &mode);
            luassert(r == 0);
            return encode_video_mode(mode);
        }
        LUNA_WINDOW_API VideoMode get_monitor_video_mode(monitor_t monitor)
        {
            Monitor* m = (Monitor*)monitor;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected monitor.");
            SDL_DisplayMode mode;
            int r = SDL_GetCurrentDisplayMode(m->m_index, &mode);
            luassert(r == 0);
            return encode_video_mode(mode);
        }
        LUNA_WINDOW_API UInt2U get_monitor_native_resolution(monitor_t monitor)
        {
            Monitor* m = (Monitor*)monitor;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected monitor.");
            SDL_DisplayMode mode;
            int r = SDL_GetDesktopDisplayMode(m->m_index, &mode);
            luassert(r == 0);
            return UInt2U((u32)mode.w, (u32)mode.h);
        }
        LUNA_WINDOW_API Int2U get_monitor_position(monitor_t monitor)
        {
            Monitor* m = (Monitor*)monitor;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected monitor.");
            SDL_Rect rect;
            int r = SDL_GetDisplayBounds(m->m_index, &rect);
            luassert(r == 0);
            return Int2U(rect.x, rect.y);
        }
        LUNA_WINDOW_API RectI get_monitor_working_area(monitor_t monitor)
        {
            Monitor* m = (Monitor*)monitor;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected monitor.");
            SDL_Rect rect;
            int r = SDL_GetDisplayUsableBounds(m->m_index, &rect);
            luassert(r == 0);
            return RectI(rect.x, rect.y, rect.w, rect.h);
        }
        LUNA_WINDOW_API Name get_monitor_name(monitor_t monitor)
        {
            Monitor* m = (Monitor*)monitor;
            return m->m_name;
        }
    }
}