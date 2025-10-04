
/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.cpp
* @author JXMaster
* @date 2025/10/2
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Display.hpp"
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#include <shellscalingapi.h>
#include <Luna/Runtime/Unicode.hpp>

#pragma comment(lib, "User32.lib")

namespace Luna
{
    namespace Window
    {
        struct DisplayQueryContext
        {
            Vector<HMONITOR> displays;
        };

        DisplayEvents g_display_events;

        static BOOL Monitorenumproc(
            HMONITOR hMonitor,
            HDC hdcMonitor,
            LPRECT lprcMonitor,
            LPARAM dwData
        )
        {
            DisplayQueryContext* ctx = (DisplayQueryContext*)dwData;
            ctx->displays.push_back(hMonitor);
            return TRUE;
        }

        static_assert(sizeof(HMONITOR) == sizeof(display_t), "Incorrect monitor handle size.");

        LUNA_WINDOW_API display_t get_primary_display()
        {
            POINT pt = { 0, 0 };
            HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            return monitor;
        }
        LUNA_WINDOW_API void get_displays(Vector<display_t>& out_displays)
        {
            DisplayQueryContext ctx;
            BOOL r = ::EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&ctx);
            luassert_msg_always(r, "EnumDisplayMonitors failed");
            for(HMONITOR monitor : ctx.displays)
            {
                out_displays.push_back(monitor);
            }
        }
        LUNA_WINDOW_API DisplayEvents& get_display_events()
        {
            return g_display_events;
        }
        LUNA_WINDOW_API RV get_display_supported_video_modes(display_t display, Vector<VideoMode>& out_video_modes)
        {
            MONITORINFOEXW info;
            memzero(&info);
            info.cbSize = sizeof(MONITORINFOEXW);
            BOOL r = ::GetMonitorInfoW((HMONITOR)display, &info);
            if(!r) return BasicError::bad_platform_call();
            DEVMODEW dev_mode;
            dev_mode.dmSize = sizeof(DEVMODEW);
            for (DWORD i = 0; EnumDisplaySettingsW(info.szDevice, i, &dev_mode); ++i)
            {
                VideoMode mode;
                mode.width = dev_mode.dmPelsWidth;
                mode.height = dev_mode.dmPelsHeight;
                mode.refresh_rate = dev_mode.dmDisplayFrequency;
                mode.bits_per_pixel = dev_mode.dmBitsPerPel;
                out_video_modes.push_back(mode);
            }
            return ok;
        }
        LUNA_WINDOW_API R<VideoMode> get_display_video_mode(display_t display)
        {
            MONITORINFOEXW info;
            memzero(&info);
            info.cbSize = sizeof(MONITORINFOEXW);
            BOOL r = ::GetMonitorInfoW((HMONITOR)display, &info);
            if(!r) return BasicError::bad_platform_call();
            DEVMODEW dev_mode;
            dev_mode.dmSize = sizeof(DEVMODEW);
            if (!EnumDisplaySettingsW(info.szDevice, ENUM_CURRENT_SETTINGS, &dev_mode))
            {
                return set_error(BasicError::bad_platform_call(), "EnumDisplaySettingsW failed");
            }
            VideoMode mode;
            mode.width = dev_mode.dmPelsWidth;
            mode.height = dev_mode.dmPelsHeight;
            mode.refresh_rate = dev_mode.dmDisplayFrequency;
            mode.bits_per_pixel = dev_mode.dmBitsPerPel;
            return mode;
        }
        LUNA_WINDOW_API R<VideoMode> get_display_native_video_mode(display_t display)
        {
            MONITORINFOEXW info;
            memzero(&info);
            info.cbSize = sizeof(MONITORINFOEXW);
            BOOL r = ::GetMonitorInfoW((HMONITOR)display, &info);
            if(!r) return BasicError::bad_platform_call();
            DEVMODEW dev_mode;
            dev_mode.dmSize = sizeof(DEVMODEW);
            if (!EnumDisplaySettingsW(info.szDevice, ENUM_REGISTRY_SETTINGS, &dev_mode))
            {
                return set_error(BasicError::bad_platform_call(), "EnumDisplaySettingsW failed");
            }
            VideoMode mode;
            mode.width = dev_mode.dmPelsWidth;
            mode.height = dev_mode.dmPelsHeight;
            mode.refresh_rate = dev_mode.dmDisplayFrequency;
            mode.bits_per_pixel = dev_mode.dmBitsPerPel;
            return mode;
        }
        LUNA_WINDOW_API R<Int2U> get_display_position(display_t display)
        {
            MONITORINFO info;
            memzero(&info);
            info.cbSize = sizeof(MONITORINFO);
            BOOL r = ::GetMonitorInfoW((HMONITOR)display, &info);
            if(!r) return BasicError::bad_platform_call();
            return Int2U(info.rcMonitor.left, info.rcMonitor.top);
        }
        LUNA_WINDOW_API R<RectI> get_display_working_area(display_t display)
        {
            MONITORINFO info;
            memzero(&info);
            info.cbSize = sizeof(MONITORINFO);
            BOOL r = ::GetMonitorInfoW((HMONITOR)display, &info);
            if(!r) return BasicError::bad_platform_call();
            return RectI(info.rcWork.left,
                info.rcWork.top,
                info.rcWork.right - info.rcWork.left,
                info.rcWork.bottom - info.rcWork.top);
        }
        LUNA_WINDOW_API R<Name> get_display_name(display_t display)
        {
            MONITORINFOEXW info;
            memzero(&info);
            info.cbSize = sizeof(MONITORINFOEXW);
            BOOL r = ::GetMonitorInfoW((HMONITOR)display, &info);
            if(!r) return BasicError::bad_platform_call();
            usize len = utf16_to_utf8_len((c16*)info.szDevice);
            StackAllocator alloc;
            c8* buf = (c8*)alloc.allocate(len + 1);
            utf16_to_utf8(buf, len + 1, (c16*)info.szDevice, CCHDEVICENAME);
            return Name(buf);
        }
    }
}