/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.cpp
* @author JXMaster
* @date 2024/6/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#include <SDL3/SDL_video.h>
#define LUNA_WINDOW_API LUNA_EXPORT

#include "Display.hpp"
#include "Common.hpp"
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    namespace Window
    {
        Vector<UniquePtr<Display>> g_displays;
        DisplayEvents g_display_events;

        VideoMode encode_video_mode(const SDL_DisplayMode& mode)
        {
            VideoMode dst_mode;
            dst_mode.width = mode.w;
            dst_mode.height = mode.h;
            dst_mode.refresh_rate = mode.refresh_rate;
            switch (mode.format)
            {
                case SDL_PIXELFORMAT_RGB332:
                    dst_mode.bits_per_pixel = 8;
                    break;
                case SDL_PIXELFORMAT_XRGB4444:
                case SDL_PIXELFORMAT_XBGR4444:
                case SDL_PIXELFORMAT_ARGB4444:
                case SDL_PIXELFORMAT_RGBA4444:
                case SDL_PIXELFORMAT_ABGR4444:
                case SDL_PIXELFORMAT_BGRA4444:
                case SDL_PIXELFORMAT_XRGB1555:
                case SDL_PIXELFORMAT_XBGR1555:
                case SDL_PIXELFORMAT_ARGB1555:
                case SDL_PIXELFORMAT_RGBA5551:
                case SDL_PIXELFORMAT_ABGR1555:
                case SDL_PIXELFORMAT_BGRA5551:
                case SDL_PIXELFORMAT_RGB565:
                case SDL_PIXELFORMAT_BGR565:
                    dst_mode.bits_per_pixel = 16;
                    break;
                case SDL_PIXELFORMAT_RGB24:
                case SDL_PIXELFORMAT_BGR24:
                    dst_mode.bits_per_pixel = 24;
                    break;
                case SDL_PIXELFORMAT_XRGB8888:
                case SDL_PIXELFORMAT_RGBX8888:
                case SDL_PIXELFORMAT_XBGR8888:
                case SDL_PIXELFORMAT_BGRX8888:
                case SDL_PIXELFORMAT_ARGB8888:
                case SDL_PIXELFORMAT_RGBA8888:
                case SDL_PIXELFORMAT_ABGR8888:
                case SDL_PIXELFORMAT_BGRA8888:
                case SDL_PIXELFORMAT_ARGB2101010:
                    dst_mode.bits_per_pixel = 32;
                    break;
                default:
                    lupanic_always();
                    dst_mode.bits_per_pixel = 32;
                    break;
            }
            return dst_mode;
        }
        RV refresh_display_list()
        {
            Vector<UniquePtr<Display>> old_displays = move(g_displays);
            int num_displays;
            SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
            if (displays == nullptr)
            {
                return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
            }
            g_displays.reserve((u32)num_displays);
            for(int i = 0; i < num_displays; ++i)
            {
                const char* name = SDL_GetDisplayName(displays[i]);
                if(!name)
                {
                    SDL_free(displays);
                    return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
                }
                Name display_name = name;
                // try to match existing display.
                UniquePtr<Display> display;
                for(auto iter = old_displays.begin(); iter != old_displays.end(); ++iter)
                {
                    if (iter->get()->m_name == display_name && !iter->get()->m_disconnected)
                    {
                        display = move(*iter);
                        old_displays.erase(iter);
                        break;
                    }
                }
                // Create new display instance if not matched.
                if (!display)
                {
                    display.reset(memnew<Display>());
                }
                // Fill display data.
                display->m_name = display_name;
                display->m_id = displays[i];
                g_displays.push_back(move(display));
            }
            SDL_free(displays);
            return ok;
        }
        display_t get_display_from_display_id(SDL_DisplayID id)
        {
            for(auto& display : g_displays)
            {
                if(display->m_id == id)
                {
                    return (display_t)display.get();
                }
            }
            return nullptr;
        }
        RV display_init()
        {
            return refresh_display_list();
        }
        void display_close()
        {
            g_displays.clear();
            g_displays.shrink_to_fit();
            g_display_events.reset();
        }
        LUNA_WINDOW_API display_t get_primary_display()
        {
            return get_display_from_display_id(SDL_GetPrimaryDisplay());
        }
        LUNA_WINDOW_API void get_displays(Vector<display_t>& out_displays)
        {
            for(auto& display : g_displays)
            {
                out_displays.push_back(display.get());
            }
        }
        LUNA_WINDOW_API DisplayEvents& get_display_events()
        {
            return g_display_events;
        }
        LUNA_WINDOW_API RV get_display_supported_video_modes(display_t display, Vector<VideoMode>& out_video_modes)
        {
            Display* m = (Display*)display;
            if(m->m_disconnected) return set_error(BasicError::not_supported(), "get_display_supported_video_modes called on a disconnected display.");
            int num_modes;
            SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(m->m_id, &num_modes);
            if(modes == nullptr) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
            for(int i = 0; i < num_modes; ++i)
            {
                VideoMode mode = encode_video_mode(*modes[i]);
                out_video_modes.push_back(mode);
            }
            SDL_free(modes);
            return ok;
        }
        LUNA_WINDOW_API R<VideoMode> get_display_video_mode(display_t display)
        {
            Display* m = (Display*)display;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected display.");
            const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(m->m_id);
            if(mode == nullptr) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
            return encode_video_mode(*mode);
        }
        LUNA_WINDOW_API R<VideoMode> get_display_native_video_mode(display_t display)
        {
            Display* m = (Display*)display;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected display.");
            const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(m->m_id);
            if(mode == nullptr) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
            return encode_video_mode(*mode);
        }
        LUNA_WINDOW_API R<Int2U> get_display_position(display_t display)
        {
            Display* m = (Display*)display;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected display.");
            SDL_Rect rect;
            lutry
            {
                luexp(encode_sdl_result(SDL_GetDisplayBounds(m->m_id, &rect)));
            }
            lucatchret;
            return Int2U(rect.x, rect.y);
        }
        LUNA_WINDOW_API R<RectI> get_display_working_area(display_t display)
        {
            Display* m = (Display*)display;
            lucheck_msg(!m->m_disconnected, "Cannot call this function on a disconnected display.");
            SDL_Rect rect;
            lutry
            {
                luexp(encode_sdl_result(SDL_GetDisplayUsableBounds(m->m_id, &rect)));
            }
            lucatchret;
            return RectI(rect.x, rect.y, rect.w, rect.h);
        }
        LUNA_WINDOW_API R<Name> get_display_name(display_t display)
        {
            Display* m = (Display*)display;
            return m->m_name;
        }
    }
}