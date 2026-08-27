/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2022/10/31
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/String.hpp>
#include "../Window.hpp"
#include "Event.hpp"
#include "Window.meta.generated.hpp"
namespace Luna
{
    namespace Window
    {
        extern StartupParams g_startup_params;
        String g_name;
        Version g_version = Version(0, 0, 0);

        struct WindowModule : public Module
        {
            virtual const c8* get_name() override { return "Window"; }
            virtual RV on_init() override
            {
                if (g_startup_params.name)
                {
                    g_name = g_startup_params.name;
                }
                else
                {
                    g_name.clear();
                }
                g_version = g_startup_params.version;
#if defined(LUNA_PLATFORM_MACOS)
                reset_application_quit_request();
#endif
                Meta::register_Window_types();
                RV result = platform_init();
                if(failed(result))
                {
                    g_name.clear();
                    g_name.shrink_to_fit();
                }
                return result;
            }
            virtual void on_close() override
            {
                platform_close();
                g_name.clear();
                g_name.shrink_to_fit();
            }
        };

        LUNA_WINDOW_API const c8* get_app_name()
        {
            return g_name.c_str();
        }

        LUNA_WINDOW_API Version get_app_version()
        {
            return g_version;
        }
        
    }
    LUNA_WINDOW_API Module* module_window()
    {
        static Window::WindowModule m;
        return &m;
    }
}
